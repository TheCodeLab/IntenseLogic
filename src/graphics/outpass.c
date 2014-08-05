#include "outpass.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "graphics/arrayattrib.h"
#include "graphics/context.h"
#include "graphics/fragdata.h"
#include "graphics/framebuffer.h"
#include "graphics/material.h"
#include "graphics/renderer.h"
#include "tgl/tgl.h"

typedef struct ilG_out {
    ilG_context *context;
    ilG_material material;
    ilG_material horizblur, vertblur;
    ilG_fbo *front, *result;
    GLuint vao, vbo, size_loc;
    unsigned w, h;
    int which;
} ilG_out;

static void out_free(void *ptr)
{
    ilG_out *self = ptr;
    ilG_material_free(&self->material);
    ilG_material_free(&self->horizblur);
    ilG_material_free(&self->vertblur);
    ilG_fbo_free(self->front);
    ilG_fbo_free(self->result);
    glDeleteBuffers(1, &self->vao);
    glDeleteBuffers(1, &self->vbo);
    free(self);
}

static void fullscreenTexture(ilG_out *self)
{
    tgl_check("Unknown");
    glBindVertexArray(self->vao);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    tgl_check("Error drawing fullscreen quad");
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
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, context->fbtextures[self->which]); // bind the framebuffer we want to display
    tgl_check("Error setting up for post processing");

    self->w = context->width;
    self->h = context->height;
    ilG_material_bind(&self->material);
    glUniform2f(self->size_loc, self->w, self->h);
    fullscreenTexture(self);

    if (context->hdr) {
        unsigned i;
        int swapped = 0;
        for (i = 0; i < 4; i++) {
            unsigned w = context->width / (1<<i),
                     h = context->height / (1<<i);
            self->w = w; self->h = h;

            // Into the front buffer,
            ilG_fbo_bind(self->front, ILG_FBO_WRITE);
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
            glBindTexture(GL_TEXTURE_RECTANGLE, ilG_fbo_getTex(self->front, 0));
            // into the result buffer,
            ilG_fbo_bind(self->result, ILG_FBO_RW);
            glViewport(0,0, w,h);
            // do a horizontal blur.
            ilG_material_bind(&self->horizblur);
            glUniform2f(self->size_loc, self->w, self->h);
            fullscreenTexture(self);

            // Into the screen,
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, context->width, context->height);
            // blend,
            glEnable(GL_BLEND);
            // additively,
            glBlendFunc(GL_ONE, GL_ONE);
            // from the result buffer,
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_RECTANGLE, ilG_fbo_getTex(self->result, 0));
            // do a vertical blur.
            ilG_material_bind(&self->vertblur);
            glUniform2f(self->size_loc, self->w, self->h);
            fullscreenTexture(self);
            glDisable(GL_BLEND);

            swapped = !swapped;
        }
        ilG_context_bindFB(context);
    }

    context->material = NULL;
    context->materialb = NULL;

    SDL_GL_SwapWindow(context->window);
    tgl_check("outpass");
}

static bool out_build(void *ptr, ilG_rendid id, ilG_context *context, ilG_buildresult *out)
{
    (void)id;
    ilG_out *self = ptr;
    self->context = context;
    if (context->hdr) {
        if (ilG_fbo_build(self->front, context)) {
            return false;
        }
        if (ilG_fbo_build(self->result, context)) {
            return false;
        }
        if (ilG_material_link(&self->horizblur, context)) {
            return false;
        }
        if (ilG_material_link(&self->vertblur, context)) {
            return false;
        }
    }
    ilG_material_fragment_file(&self->material, context->hdr? "hdr.frag" : "post.frag");
    if (ilG_material_link(&self->material, context)) {
        return false;
    }
    self->size_loc = ilG_material_getLoc(&self->material, "size");
    static const float data[] = {
        0, 0,
        0, 1,
        1, 1,
        1, 0
    };
    tgl_check("Unknown");
    glGenVertexArrays(1, &self->vao);
    glBindVertexArray(self->vao);
    glGenBuffers(1, &self->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
    glVertexAttribPointer(ILG_ARRATTR_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribPointer(ILG_ARRATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
    glEnableVertexAttribArray(ILG_ARRATTR_TEXCOORD);
    tgl_check("Failed to upload quad");

    *out = (ilG_buildresult) {
        .free = out_free,
        .update = out_update,
        .draw = NULL,
        .view = NULL,
        .types = NULL,
        .num_types = 0,
        .obj = self
    };
    return true;
}

ilG_builder ilG_out_builder()
{
    ilG_out *self = calloc(1, sizeof(ilG_out));
    ilG_material *m;

    ilG_fbo *f = self->front = ilG_fbo_new();
    ilG_fbo_numTargets(f, 1);
    ilG_fbo_name(f, ilG_fbo_self, "Blur Front Buffer");
    ilG_fbo_size_rel(f, 0, 1.0, 1.0);
    ilG_fbo_texture(f, 0, GL_TEXTURE_RECTANGLE, GL_RGB, GL_COLOR_ATTACHMENT0);

    f = self->result = ilG_fbo_new();
    ilG_fbo_numTargets(f, 1);
    ilG_fbo_name(f, ilG_fbo_self, "Horizontal Result Buffer");
    ilG_fbo_size_rel(f, 0, 1.0, 1.0);
    ilG_fbo_texture(f, 0, GL_TEXTURE_RECTANGLE, GL_RGB, GL_COLOR_ATTACHMENT0);

    ilG_material_init(&self->horizblur);
    ilG_material_init(&self->vertblur);
    ilG_material_init(&self->material);

    m = &self->horizblur;
    ilG_material_vertex_file(m, "post.vert");
    ilG_material_name(m, "Horizontal Blur Shader");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_fragData(m, 0, "out_Color");
    ilG_material_textureUnit(m, 0, "tex");
    ilG_material_fragment_file(m, "horizblur.frag");

    m = &self->vertblur;
    ilG_material_vertex_file(m, "post.vert");
    ilG_material_name(m, "Vertical Blur Shader");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_fragData(m, 0, "out_Color");
    ilG_material_textureUnit(m, 0, "tex");
    ilG_material_fragment_file(m, "vertblur.frag");

    m = &self->material;
    ilG_material_vertex_file(m, "post.vert");
    ilG_material_name(m, "Tone Mapping Shader");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_textureUnit(m, 0, "tex");

    tgl_check("Failed to build vbo");
    self->which = 1;

    return ilG_builder_wrap(self, out_build);
}
