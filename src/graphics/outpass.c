#include "graphics/renderer.h"

#include "tgl/tgl.h"
#include "graphics/arrayattrib.h"
#include "graphics/context.h"
#include "graphics/material.h"

enum {
    OUT_POSITION
};

void ilG_out_free(ilG_out *out)
{
    ilG_renderman_delMaterial(out->rm, out->tonemap);
    ilG_renderman_delMaterial(out->rm, out->horizblur);
    ilG_renderman_delMaterial(out->rm, out->vertblur);
    tgl_fbo_free(&out->front);
    tgl_fbo_free(&out->result);
    tgl_quad_free(&out->quad);
    tgl_vao_free(&out->vao);
}

static void out_free(void *ptr)
{
    ilG_out_free(ptr);
}

static void out_bloom(ilG_out *self)
{
    ilG_context *context = self->context;
    unsigned i;
    int swapped = 0;

    ilG_material *tonemap = ilG_renderman_findMaterial(self->rm, self->tonemap);
    ilG_material *horizblur = ilG_renderman_findMaterial(self->rm, self->horizblur);
    ilG_material *vertblur = ilG_renderman_findMaterial(self->rm, self->vertblur);

    tgl_vao_bind(&self->vao);

    glActiveTexture(GL_TEXTURE0);
    // bind the framebuffer we want to display
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    tgl_fbo_bindTex(&context->accum, 0);
    ilG_material_bind(tonemap);
    glUniform2f(self->t_size_loc, self->w, self->h);
    glUniform1f(self->t_exposure_loc, self->exposure);
    glUniform1f(self->gamma_loc, self->gamma);
    tgl_quad_draw_once(&self->quad);

    for (i = 0; i < 4; i++) {
        unsigned w = context->width / (1<<i),
            h = context->height / (1<<i);
        self->w = w; self->h = h;

        // Into the front buffer,
        tgl_fbo_bind(&self->front, TGL_FBO_WRITE);
        // from the accumulation buffer,
        tgl_fbo_bind(&context->accum, TGL_FBO_READ);
        // downscale.
        glBlitFramebuffer(0,0, context->width,context->height,
                          0,0, w,h,
                          GL_COLOR_BUFFER_BIT,
                          GL_LINEAR);
        tgl_check("Blit failed");

        // From the front buffer,
        glActiveTexture(GL_TEXTURE0);
        tgl_fbo_bindTex(&self->front, 0);
        // into the result buffer,
        tgl_fbo_bind(&self->result, TGL_FBO_RW);
        glViewport(0,0, w,h);
        // do a horizontal blur.
        ilG_material_bind(horizblur);
        glUniform2f(self->h_size_loc, self->w, self->h);
        glUniform1f(self->h_exposure_loc, self->exposure);
        tgl_quad_draw_once(&self->quad);

        // Into the screen,
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, context->width, context->height);
        // blend,
        glEnable(GL_BLEND);
        // additively,
        glBlendFunc(GL_ONE, GL_ONE);
        // from the result buffer,
        glActiveTexture(GL_TEXTURE0);
        tgl_fbo_bindTex(&self->result, 0);
        // do a vertical blur.
        ilG_material_bind(vertblur);
        glUniform2f(self->v_size_loc, self->w, self->h);
        tgl_quad_draw_once(&self->quad);
        glDisable(GL_BLEND);

        swapped = !swapped;
    }
}

static void out_update(void *ptr, ilG_rendid id)
{
    (void)id;
    ilG_out *self = ptr;
    ilG_context *context = self->context;

    tgl_check("Unknown");
    // prepare the GL state for outputting to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (context->debug_render) {
        glClearColor(0.196, 0.804, 0.196, 1.0); // lime green
    } else {
        glClearColor(0, 0, 0, 1.0);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    tgl_check("Error setting up for post processing");
    self->w = context->width;
    self->h = context->height;

    if (context->hdr) {
        out_bloom(self);
    } else {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        tgl_fbo_bind(&context->accum, TGL_FBO_READ);
        unsigned w = context->width, h = context->height;
        glBlitFramebuffer(0,0, w,h,
                          0,0, w,h,
                          GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
    }

    SDL_GL_SwapWindow(context->window);
    tgl_check("outpass");
}

void ilG_out_resize(ilG_out *out, unsigned w, unsigned h)
{
    tgl_fbo_build(&out->front, w, h);
    tgl_fbo_build(&out->result, w, h);
}

static void out_resize(const il_value *data, il_value *ctx)
{
    ilG_out *self = il_value_tomvoid(ctx);
    const il_vector *vec = il_value_tovec(data);
    unsigned w = il_vector_geti(vec, 0);
    unsigned h = il_vector_geti(vec, 1);
    ilG_out_resize(self, w, h);
}

bool ilG_out_build(ilG_out *out, ilG_context *context, char **error)
{
    tgl_fbo *f;
    GLenum fmt = context->msaa? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_RECTANGLE;

    f = &out->front;
    tgl_fbo_init(f);
    tgl_fbo_numTargets(f, 1);
    tgl_fbo_texture(f, 0, fmt, GL_RGB8, GL_RGB, GL_COLOR_ATTACHMENT0, GL_UNSIGNED_BYTE);

    f = &out->result;
    tgl_fbo_init(f);
    tgl_fbo_numTargets(f, 1);
    tgl_fbo_texture(f, 0, fmt, GL_RGB8, GL_RGB, GL_COLOR_ATTACHMENT0, GL_UNSIGNED_BYTE);

    if (context->msaa) {
        tgl_fbo_multisample(&out->front, 0, context->msaa, false);
        tgl_fbo_multisample(&out->result, 0, context->msaa, false);
    }
    if (!tgl_fbo_build(&out->front, context->width, context->height)) {
        return false;
    }
    if (!tgl_fbo_build(&out->result, context->width, context->height)) {
        return false;
    }

    ilG_material m, *mat;

    ilG_material_init(&m);
    ilG_material_name(&m, "Horizontal Blur Shader");
    ilG_material_arrayAttrib(&m, OUT_POSITION, "in_Texcoord");
    ilG_material_fragData(&m, 0, "out_Color");
    ilG_material_textureUnit(&m, 0, "tex");
    if (!ilG_renderman_addMaterialFromFile(out->rm, m, "post.vert", "horizblur.frag", &out->horizblur, error)) {
        return false;
    }
    mat = ilG_renderman_findMaterial(out->rm, out->horizblur);
    out->h_exposure_loc = ilG_material_getLoc(mat, "exposure");
    out->h_size_loc = ilG_material_getLoc(mat, "size");

    ilG_material_init(&m);
    ilG_material_name(&m, "Vertical Blur Shader");
    ilG_material_arrayAttrib(&m, OUT_POSITION, "in_Texcoord");
    ilG_material_fragData(&m, 0, "out_Color");
    ilG_material_textureUnit(&m, 0, "tex");
    if (!ilG_renderman_addMaterialFromFile(out->rm, m, "post.vert", "vertblur.frag", &out->vertblur, error)) {
        return false;
    }
    mat = ilG_renderman_findMaterial(out->rm, out->vertblur);
    out->v_size_loc = ilG_material_getLoc(mat, "size");

    ilG_material_init(&m);
    ilG_material_name(&m, "Tone Mapping Shader");
    ilG_material_arrayAttrib(&m, OUT_POSITION, "in_Texcoord");
    ilG_material_textureUnit(&m, 0, "tex");
    ilG_material_fragData(&m, 0, "out_Color");
    if (!ilG_renderman_addMaterialFromFile(out->rm, m, "post.vert", "hdr.frag", &out->tonemap, error)) {
        return false;
    }
    mat = ilG_renderman_findMaterial(out->rm, out->tonemap);
    out->t_size_loc = ilG_material_getLoc(mat, "size");
    out->t_exposure_loc = ilG_material_getLoc(mat, "exposure");
    out->gamma_loc = ilG_material_getLoc(mat, "gamma");

    tgl_vao_init(&out->vao);
    tgl_vao_bind(&out->vao);
    tgl_quad_init(&out->quad, OUT_POSITION);

    il_storage_void sv = {out, NULL};
    ilE_register(&context->resize, ILE_DONTCARE, ILE_ANY, out_resize, il_value_opaque(sv));

    return true;
}

static bool out_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    ilG_out *self = ptr;
    ilG_context *context = self->context;
    self->rm = rm;
    if (context->hdr && !ilG_out_build(self, context, &out->error)) {
        return false;
    }

    *out = (ilG_buildresult) {
        .free = out_free,
        .update = out_update,
        .draw = NULL,
        .view = NULL,
        .types = NULL,
        .num_types = 0,
        .obj = self,
        .name = strdup("Screen Output")
    };
    return true;
}

ilG_builder ilG_out_builder(ilG_out *out, ilG_context *context)
{
    out->context = context;
    return ilG_builder_wrap(out, out_build);
}
