#include "graphics/renderer.h"

#include <stdio.h>

#include "tgl/tgl.h"
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
    ilG_renderman *rm = tm->rm;
    unsigned i;
    int swapped = 0;

    ilG_material *tonemap = ilG_renderman_findMaterial(rm, tm->tonemap);
    ilG_material *horizblur = ilG_renderman_findMaterial(rm, tm->horizblur);
    ilG_material *vertblur = ilG_renderman_findMaterial(rm, tm->vertblur);

    tm->w = rm->width;
    tm->h = rm->height;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (tm->debug_render) {
        glClearColor(0.196f, 0.804f, 0.196f, 1.0f); // lime green
    } else {
        glClearColor(0.f, 0.f, 0.f, 1.f);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    tgl_vao_bind(&tm->vao);

    glActiveTexture(GL_TEXTURE0);
    // bind the framebuffer we want to display
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    tgl_fbo_bindTex(&rm->accum, 0);
    ilG_material_bind(tonemap);
    glUniform2f(tm->t_size_loc, tm->w, tm->h);
    glUniform1f(tm->t_exposure_loc, tm->exposure);
    glUniform1f(tm->gamma_loc, tm->gamma);
    tgl_quad_draw_once(&tm->quad);

    for (i = 0; i < 4; i++) {
        char buf[64];
        snprintf(buf, 64, "Iteration %i", i);
        #ifndef __APPLE__
        glPushDebugGroup(GL_DEBUG_SOURCE_THIRD_PARTY, 0, -1, buf);
        #endif

        unsigned w = rm->width / (1<<i),
            h = rm->height / (1<<i);
        tm->w = w; tm->h = h;

        // Into the front buffer,
        tgl_fbo_bind(&tm->front, TGL_FBO_WRITE);
        // from the accumulation buffer,
        tgl_fbo_bind(&rm->accum, TGL_FBO_READ);
        // downscale.
        glBlitFramebuffer(0,0, rm->width,rm->height,
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
        glViewport(0, 0, rm->width, rm->height);
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

        #ifndef __APPLE__
        glPopDebugGroup();
        #endif

        swapped = !swapped;
    }
}

void ilG_tonemapper_resize(ilG_tonemapper *tonemapper, unsigned w, unsigned h)
{
    tgl_fbo_build(&tonemapper->front, w, h);
    tgl_fbo_build(&tonemapper->result, w, h);
}

bool ilG_tonemapper_build(ilG_tonemapper *tm, ilG_renderman *rm, bool msaa, char **error)
{
    memset(tm, 0, sizeof(*tm));
    tm->rm = rm;
    tm->msaa = msaa;
    tgl_fbo *f;
    GLenum fmt = msaa? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_RECTANGLE;

    f = &tm->front;
    tgl_fbo_init(f);
    tgl_fbo_numTargets(f, 1);
    tgl_fbo_texture(f, 0, fmt, GL_RGB8, GL_RGB, GL_COLOR_ATTACHMENT0, GL_UNSIGNED_BYTE);

    f = &tm->result;
    tgl_fbo_init(f);
    tgl_fbo_numTargets(f, 1);
    tgl_fbo_texture(f, 0, fmt, GL_RGB8, GL_RGB, GL_COLOR_ATTACHMENT0, GL_UNSIGNED_BYTE);

    if (msaa) {
        tgl_fbo_multisample(&tm->front, 0, msaa, false);
        tgl_fbo_multisample(&tm->result, 0, msaa, false);
    }
    if (!tgl_fbo_build(&tm->front, rm->width, rm->height)) {
        return false;
    }
    if (!tgl_fbo_build(&tm->result, rm->width, rm->height)) {
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
