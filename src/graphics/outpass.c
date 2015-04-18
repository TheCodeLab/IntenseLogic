#include "graphics/renderer.h"

#include "tgl/tgl.h"
#include "graphics/arrayattrib.h"
#include "graphics/context.h"
#include "graphics/fragdata.h"
#include "graphics/material.h"

typedef struct ilG_out {
    ilG_context *context;
    ilG_renderman *rm;
    ilG_matid tonemap, horizblur, vertblur;
    tgl_fbo front, result;
    tgl_quad quad;
    tgl_vao vao;
    GLuint size_loc;
    unsigned w, h;
    int which;
} ilG_out;

enum {
    OUT_POSITION
};

static void out_free(void *ptr)
{
    ilG_out *self = ptr;
    ilG_renderman_delMaterial(self->rm, self->tonemap);
    ilG_renderman_delMaterial(self->rm, self->horizblur);
    ilG_renderman_delMaterial(self->rm, self->vertblur);
    tgl_fbo_free(&self->front);
    tgl_fbo_free(&self->result);
    tgl_quad_free(&self->quad);
    tgl_vao_free(&self->vao);
    free(self);
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
    tgl_fbo_bindTex(&context->fb, self->which);
    ilG_material_bind(tonemap);
    glUniform2f(self->size_loc, self->w, self->h);
    tgl_quad_draw_once(&self->quad);

    for (i = 0; i < 4; i++) {
        unsigned w = context->width / (1<<i),
            h = context->height / (1<<i);
        self->w = w; self->h = h;

        // Into the front buffer,
        tgl_fbo_bind(&self->front, TGL_FBO_WRITE);
        // from the context,
        ilG_context_bind_for_outpass(context);
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
        glUniform2f(self->size_loc, self->w, self->h);
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
        glUniform2f(self->size_loc, self->w, self->h);
        tgl_quad_draw_once(&self->quad);
        glDisable(GL_BLEND);

        swapped = !swapped;
    }
    ilG_context_bindFB(context);
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
        ilG_context_bind_for_outpass(context);
        unsigned w = context->width, h = context->height;
        glBlitFramebuffer(0,0, w,h,
                          0,0, w,h,
                          GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
    }

    SDL_GL_SwapWindow(context->window);
    tgl_check("outpass");
}

static void out_resize(const il_value *data, il_value *ctx)
{
    ilG_out *self = il_value_tomvoid(ctx);
    const il_vector *vec = il_value_tovec(data);
    unsigned w = il_vector_geti(vec, 0);
    unsigned h = il_vector_geti(vec, 1);
    tgl_fbo_build(&self->front, w, h);
    tgl_fbo_build(&self->result, w, h);
}

static bool out_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    ilG_out *self = ptr;
    ilG_context *context = self->context;
    self->rm = rm;
    if (context->hdr) {
        tgl_fbo *f;
        GLenum fmt = context->msaa? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_RECTANGLE;

        f = &self->front;
        tgl_fbo_numTargets(f, 1);
        tgl_fbo_texture(f, 0, fmt, GL_RGB8, GL_RGB, GL_COLOR_ATTACHMENT0, GL_UNSIGNED_BYTE);

        f = &self->result;
        tgl_fbo_numTargets(f, 1);
        tgl_fbo_texture(f, 0, fmt, GL_RGB8, GL_RGB, GL_COLOR_ATTACHMENT0, GL_UNSIGNED_BYTE);

        if (context->msaa) {
            tgl_fbo_multisample(&self->front, 0, context->msaa, false);
            tgl_fbo_multisample(&self->result, 0, context->msaa, false);
        }
        if (!tgl_fbo_build(&self->front, context->width, context->height)) {
            return false;
        }
        if (!tgl_fbo_build(&self->result, context->width, context->height)) {
            return false;
        }

        ilG_material m;

        ilG_material_init(&m);
        ilG_material_name(&m, "Horizontal Blur Shader");
        ilG_material_arrayAttrib(&m, OUT_POSITION, "in_Texcoord");
        ilG_material_fragData(&m, 0, "out_Color");
        ilG_material_textureUnit(&m, 0, "tex");
        if (!ilG_material_vertex_file(&m, "post.vert", &out->error)) {
            return false;
        }
        if (!ilG_material_fragment_file(&m, "horizblur.frag", &out->error)) {
            return false;
        }
        if (!ilG_material_link(&m, &out->error)) {
            return false;
        }
        self->horizblur = ilG_renderman_addMaterial(self->rm, m);

        ilG_material_init(&m);
        ilG_material_name(&m, "Vertical Blur Shader");
        ilG_material_arrayAttrib(&m, OUT_POSITION, "in_Texcoord");
        ilG_material_fragData(&m, 0, "out_Color");
        ilG_material_textureUnit(&m, 0, "tex");
        if (!ilG_material_vertex_file(&m, "post.vert", &out->error)) {
            return false;
        }
        if (!ilG_material_fragment_file(&m, "vertblur.frag", &out->error)) {
            return false;
        }
        if (!ilG_material_link(&m, &out->error)) {
            return false;
        }
        self->vertblur = ilG_renderman_addMaterial(self->rm, m);

        ilG_material_init(&m);
        ilG_material_name(&m, "Tone Mapping Shader");
        ilG_material_arrayAttrib(&m, OUT_POSITION, "in_Texcoord");
        ilG_material_textureUnit(&m, 0, "tex");
        if (!ilG_material_vertex_file(&m, "post.vert", &out->error)) {
            return false;
        }
        if (!ilG_material_fragment_file(&m, "hdr.frag", &out->error)) {
            return false;
        }
        if (!ilG_material_link(&m, &out->error)) {
            return false;
        }
        self->size_loc = ilG_material_getLoc(&m, "size");
        self->tonemap = ilG_renderman_addMaterial(self->rm, m);

        tgl_vao_init(&self->vao);
        tgl_vao_bind(&self->vao);
        tgl_quad_init(&self->quad, OUT_POSITION);

        il_storage_void sv = {self, NULL};
        ilE_register(&context->resize, ILE_DONTCARE, ILE_ANY, out_resize, il_value_opaque(sv));
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

ilG_builder ilG_out_builder(ilG_context *context)
{
    ilG_out *self = calloc(1, sizeof(ilG_out));

    tgl_fbo_init(&self->front);
    tgl_fbo_init(&self->result);
    self->context = context;
    self->which = 1;

    return ilG_builder_wrap(self, out_build);
}
