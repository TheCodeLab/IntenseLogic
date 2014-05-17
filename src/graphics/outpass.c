#include "outpass.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "graphics/renderer.h"
#include "graphics/context.h"
#include "graphics/material.h"
#include "graphics/glutil.h"
#include "graphics/arrayattrib.h"
#include "graphics/framebuffer.h"
#include "graphics/fragdata.h"

typedef struct ilG_out {
    ilG_context *context;
    ilG_material *material;
    ilG_material *horizblur, *vertblur;
    ilG_fbo *front, *result;
    GLuint vao, vbo;
    unsigned w, h;
    int which;
} ilG_out;

static void out_free(void *ptr, ilG_rendid id)
{
    (void)id;
    ilG_out *self = ptr;
    il_unref(self->material);
    il_unref(self->horizblur);
    il_unref(self->vertblur);
    ilG_fbo_free(self->front);
    ilG_fbo_free(self->result);
    glDeleteBuffers(1, &self->vao);
    glDeleteBuffers(1, &self->vbo);
    free(self);
}

static void fullscreenTexture(ilG_out *self)
{
    ilG_testError("Unknown");
    glBindVertexArray(self->vao);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    ilG_testError("Error drawing fullscreen quad");
}

static void size_uniform(ilG_material *self, GLint location, void *user)
{
    (void)self;
    ilG_out *out = user;
    glUniform2f(location, out->w, out->h);
}

static void out_draw(void *ptr, ilG_rendid id)
{
    (void)id;
    ilG_out *self = ptr;
    ilG_context *context = self->context;

    ilG_testError("Unknown");
    ilG_bindable_unbind(context->materialb, context->material);
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
    ilG_testError("Error setting up for post processing");

    self->w = context->width;
    self->h = context->height;
    // no action() as we don't deal with positionables here
    ilG_bindable_bind(&ilG_material_bindable, self->material);
    fullscreenTexture(self);
    ilG_bindable_unbind(&ilG_material_bindable, self->material);

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
            ilG_testError("Blit failed");

            // From the front buffer,
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_RECTANGLE, ilG_fbo_getTex(self->front, 0));
            // into the result buffer,
            ilG_fbo_bind(self->result, ILG_FBO_RW);
            glViewport(0,0, w,h);
            // do a horizontal blur.
            ilG_bindable_bind(&ilG_material_bindable, self->horizblur);
            fullscreenTexture(self);
            ilG_bindable_unbind(&ilG_material_bindable, self->horizblur);

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
            ilG_bindable_bind(&ilG_material_bindable, self->vertblur);
            fullscreenTexture(self);
            ilG_bindable_unbind(&ilG_material_bindable, self->vertblur);
            glDisable(GL_BLEND);

            swapped = !swapped;
        }
        ilG_context_bindFB(context);
    }

    context->material = NULL;
    context->materialb = NULL;

    SDL_GL_SwapWindow(context->window);
    ilG_testError("outpass");
}

static bool out_build(void *ptr, ilG_rendid id, ilG_context *context, ilG_renderer *out)
{
    ilG_out *self = ptr;
    self->context = context;
    if (context->hdr) {
        if (ilG_fbo_build(self->front, context)) {
            return false;
        }
        if (ilG_fbo_build(self->result, context)) {
            return false;
        }
        if (ilG_material_link(self->horizblur, context)) {
            return false;
        }
        if (ilG_material_link(self->vertblur, context)) {
            return false;
        }
    }
    ilG_material_fragment_file(self->material, context->hdr? "hdr.frag" : "post.frag");
    if (ilG_material_link(self->material, context)) {
        return false;
    }
    static const float data[] = {
        0, 0,
	0, 1,
	1, 1,
	1, 0
    };
    ilG_testError("Unknown");
    glGenVertexArrays(1, &self->vao);
    glBindVertexArray(self->vao);
    glGenBuffers(1, &self->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
    glVertexAttribPointer(ILG_ARRATTR_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribPointer(ILG_ARRATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
    glEnableVertexAttribArray(ILG_ARRATTR_TEXCOORD);
    ilG_testError("Failed to upload quad");

    *out = (ilG_renderer) {
        .id = id,
        .free = out_free,
        .draw = out_draw,
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

    m = self->horizblur = ilG_material_new();
    ilG_material_vertex_file(m, "post.vert");
    ilG_material_name(m, "Horizontal Blur Shader");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_fragData(m, 0, "out_Color");
    ilG_material_bindFunc(m, size_uniform, self, "size");
    ilG_material_textureUnit(m, 0, "tex");
    ilG_material_fragment_file(m, "horizblur.frag");

    m = self->vertblur = ilG_material_new();
    ilG_material_vertex_file(m, "post.vert");
    ilG_material_name(m, "Vertical Blur Shader");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_fragData(m, 0, "out_Color");
    ilG_material_bindFunc(m, size_uniform, self, "size");
    ilG_material_textureUnit(m, 0, "tex");
    ilG_material_fragment_file(m, "vertblur.frag");
    
    m = self->material = ilG_material_new();
    ilG_material_vertex_file(m, "post.vert");
    ilG_material_name(m, "Tone Mapping Shader");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_bindFunc(m, size_uniform, self, "size");
    ilG_material_textureUnit(m, 0, "tex");

    ilG_testError("Failed to build vbo");
    self->which = 1;

    return ilG_builder_wrap(self, out_build);
}

