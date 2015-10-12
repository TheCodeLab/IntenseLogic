#include "graphics/renderer.h"

#include "tgl/tgl.h"
#include "graphics/arrayattrib.h"
#include "graphics/context.h"
#include "graphics/material.h"

enum {
    OUT_POSITION
};

void ilG_tonemapper_free(ilG_tonemapper *tm)
{
    ilG_renderman_delMaterial(tm->rm, tm->tonemap);
    ilG_renderman_delMaterial(tm->rm, tm->horizblur);
    ilG_renderman_delMaterial(tm->rm, tm->vertblur);
    tgl_fbo_free(&tm->front);
    tgl_fbo_free(&tm->result);
    tgl_quad_free(&tm->quad);
    tgl_vao_free(&tm->vao);
}

void ilG_tonemapper_draw(ilG_tonemapper *tm)
{
    ilG_context *context = tm->context;
    unsigned i;
    int swapped = 0;

    ilG_material *tonemap = ilG_renderman_findMaterial(tm->rm, tm->tonemap);
    ilG_material *horizblur = ilG_renderman_findMaterial(tm->rm, tm->horizblur);
    ilG_material *vertblur = ilG_renderman_findMaterial(tm->rm, tm->vertblur);

    tm->w = context->width;
    tm->h = context->height;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (context->debug_render) {
        glClearColor(0.196, 0.804, 0.196, 1.0); // lime green
    } else {
        glClearColor(0, 0, 0, 1.0);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    tgl_vao_bind(&tm->vao);

    glActiveTexture(GL_TEXTURE0);
    // bind the framebuffer we want to display
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    tgl_fbo_bindTex(&context->accum, 0);
    ilG_material_bind(tonemap);
    glUniform2f(tm->t_size_loc, tm->w, tm->h);
    glUniform1f(tm->t_exposure_loc, tm->exposure);
    glUniform1f(tm->gamma_loc, tm->gamma);
    tgl_quad_draw_once(&tm->quad);

    for (i = 0; i < 4; i++) {
        char buf[64];
        sprintf(buf, "Iteration %i", i);
        glPushDebugGroup(GL_DEBUG_SOURCE_THIRD_PARTY, 0, -1, buf);

        unsigned w = context->width / (1<<i),
            h = context->height / (1<<i);
        tm->w = w; tm->h = h;

        // Into the front buffer,
        tgl_fbo_bind(&tm->front, TGL_FBO_WRITE);
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
        tgl_fbo_bindTex(&tm->front, 0);
        // into the result buffer,
        tgl_fbo_bind(&tm->result, TGL_FBO_RW);
        glViewport(0,0, w,h);
        // do a horizontal blur.
        ilG_material_bind(horizblur);
        glUniform2f(tm->h_size_loc, tm->w, tm->h);
        glUniform1f(tm->h_exposure_loc, tm->exposure);
        tgl_quad_draw_once(&tm->quad);

        // Into the screen,
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, context->width, context->height);
        // blend,
        glEnable(GL_BLEND);
        // additively,
        glBlendFunc(GL_ONE, GL_ONE);
        // from the result buffer,
        glActiveTexture(GL_TEXTURE0);
        tgl_fbo_bindTex(&tm->result, 0);
        // do a vertical blur.
        ilG_material_bind(vertblur);
        glUniform2f(tm->v_size_loc, tm->w, tm->h);
        tgl_quad_draw_once(&tm->quad);
        glDisable(GL_BLEND);
        glPopDebugGroup();

        swapped = !swapped;
    }
}

static void tonemapper_update(void *ptr, ilG_rendid id)
{
    (void)id;
    ilG_tonemapper *self = ptr;
    ilG_tonemapper_draw(self);
}

void ilG_tonemapper_resize(ilG_tonemapper *tonemapper, unsigned w, unsigned h)
{
    tgl_fbo_build(&tonemapper->front, w, h);
    tgl_fbo_build(&tonemapper->result, w, h);
}

bool ilG_tonemapper_build(ilG_tonemapper *tm, ilG_context *context, char **error)
{
    memset(tm, 0, sizeof(*tm));
    tm->context = context;
    tm->rm = &context->manager;
    tgl_fbo *f;
    GLenum fmt = context->msaa? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_RECTANGLE;

    f = &tm->front;
    tgl_fbo_init(f);
    tgl_fbo_numTargets(f, 1);
    tgl_fbo_texture(f, 0, fmt, GL_RGB8, GL_RGB, GL_COLOR_ATTACHMENT0, GL_UNSIGNED_BYTE);

    f = &tm->result;
    tgl_fbo_init(f);
    tgl_fbo_numTargets(f, 1);
    tgl_fbo_texture(f, 0, fmt, GL_RGB8, GL_RGB, GL_COLOR_ATTACHMENT0, GL_UNSIGNED_BYTE);

    if (context->msaa) {
        tgl_fbo_multisample(&tm->front, 0, context->msaa, false);
        tgl_fbo_multisample(&tm->result, 0, context->msaa, false);
    }
    if (!tgl_fbo_build(&tm->front, context->width, context->height)) {
        return false;
    }
    if (!tgl_fbo_build(&tm->result, context->width, context->height)) {
        return false;
    }

    ilG_material m, *mat;

    ilG_material_init(&m);
    ilG_material_name(&m, "Horizontal Blur Shader");
    ilG_material_arrayAttrib(&m, OUT_POSITION, "in_Texcoord");
    ilG_material_fragData(&m, 0, "tm_Color");
    ilG_material_textureUnit(&m, 0, "tex");
    if (!ilG_renderman_addMaterialFromFile(tm->rm, m, "post.vert", "horizblur.frag", &tm->horizblur, error)) {
        return false;
    }
    mat = ilG_renderman_findMaterial(tm->rm, tm->horizblur);
    tm->h_exposure_loc = ilG_material_getLoc(mat, "exposure");
    tm->h_size_loc = ilG_material_getLoc(mat, "size");

    ilG_material_init(&m);
    ilG_material_name(&m, "Vertical Blur Shader");
    ilG_material_arrayAttrib(&m, OUT_POSITION, "in_Texcoord");
    ilG_material_fragData(&m, 0, "tm_Color");
    ilG_material_textureUnit(&m, 0, "tex");
    if (!ilG_renderman_addMaterialFromFile(tm->rm, m, "post.vert", "vertblur.frag", &tm->vertblur, error)) {
        return false;
    }
    mat = ilG_renderman_findMaterial(tm->rm, tm->vertblur);
    tm->v_size_loc = ilG_material_getLoc(mat, "size");

    ilG_material_init(&m);
    ilG_material_name(&m, "Tone Mapping Shader");
    ilG_material_arrayAttrib(&m, OUT_POSITION, "in_Texcoord");
    ilG_material_textureUnit(&m, 0, "tex");
    ilG_material_fragData(&m, 0, "tm_Color");
    if (!ilG_renderman_addMaterialFromFile(tm->rm, m, "post.vert", "hdr.frag", &tm->tonemap, error)) {
        return false;
    }
    mat = ilG_renderman_findMaterial(tm->rm, tm->tonemap);
    tm->t_size_loc = ilG_material_getLoc(mat, "size");
    tm->t_exposure_loc = ilG_material_getLoc(mat, "exposure");
    tm->gamma_loc = ilG_material_getLoc(mat, "gamma");

    tgl_vao_init(&tm->vao);
    tgl_vao_bind(&tm->vao);
    tgl_quad_init(&tm->quad, OUT_POSITION);

    return true;
}

static bool tonemapper_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *tonemapper)
{
    (void)id, (void)rm;
    ilG_tonemapper *self = ptr;
    ilG_context *context = self->context;
    if (!ilG_tonemapper_build(self, context, &tonemapper->error)) {
        return false;
    }

    *tonemapper = (ilG_buildresult) {
        .update = tonemapper_update,
        .draw = NULL,
        .view = NULL,
        .types = NULL,
        .num_types = 0,
        .obj = self,
        .name = strdup("Screen Tonemapperput")
    };
    return true;
}

ilG_builder ilG_tonemapper_builder(ilG_tonemapper *tonemapper, ilG_context *context)
{
    tonemapper->context = context;
    return ilG_builder_wrap(tonemapper, tonemapper_build);
}
