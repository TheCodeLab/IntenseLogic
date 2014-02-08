#include "outpass.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "graphics/stage.h"
#include "graphics/context.h"
#include "graphics/material.h"
#include "graphics/glutil.h"
#include "graphics/arrayattrib.h"
#include "graphics/textureunit.h"
#include "graphics/framebuffer.h"
#include "graphics/fragdata.h"

struct ilG_out {
    ilG_context *context;
    ilG_material *material;
    ilG_material *horizblur, *vertblur;
    ilG_fbo *front, *result;
    GLuint vao, vbo;
    unsigned w, h;
    int which;
};

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

static void out_run(void *ptr)
{
    ilG_out *self = ptr;
    ilG_context *context = self->context;

    if (glfwWindowShouldClose(context->window)) {
        //ilE_globalevent(il_registry, "shutdown", 0, NULL);
        return;
    }
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

            // From the front buffer,
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

    glfwSwapBuffers(context->window);
    ilG_testError("outpass");
}

const ilG_stagable ilG_out_stage = {
    .run = out_run,
    .name = "Screen Output"
};

ilG_out *ilG_out_new(struct ilG_context *context)
{
    ilG_out *self = calloc(1, sizeof(ilG_out));
    ilG_material *m;
    
    self->context = context;

    if (context->hdr) {
        ilG_fbo *f = self->front = ilG_fbo_new();
        ilG_fbo_numTargets(f, 1);
        ilG_fbo_name(f, ilG_fbo_self, "Blur Front Buffer");
        ilG_fbo_size_rel(f, 0, 1.0, 1.0);
        ilG_fbo_texture(f, 0, GL_TEXTURE_RECTANGLE, GL_RGB, GL_COLOR_ATTACHMENT0);
        if (ilG_fbo_build(f, context)) {
            return NULL;
        }

        f = self->result = ilG_fbo_new();
        ilG_fbo_numTargets(f, 1);
        ilG_fbo_name(f, ilG_fbo_self, "Horizontal Result Buffer");
        ilG_fbo_size_rel(f, 0, 1.0, 1.0);
        ilG_fbo_texture(f, 0, GL_TEXTURE_RECTANGLE, GL_RGB, GL_COLOR_ATTACHMENT0);
        if (ilG_fbo_build(f, context)) {
            return NULL;
        }

        m = ilG_material_new();
        ilG_material_vertex_file(m, "post.vert");
        ilG_material_name(m, "Horizontal Blur Shader");
        ilG_material_arrayAttrib(m, ILG_ARRATTR_POSITION, "in_Position");
        ilG_material_arrayAttrib(m, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
        ilG_material_fragData(m, 0, "out_Color");
        ilG_material_bindFunc(m, size_uniform, self, "size");
        ilG_material_textureUnit(m, ILG_TUNIT_NONE, "tex");
        ilG_material_fragment_file(m, "horizblur.frag");
        if (ilG_material_link(m, context)) {
            return NULL;
        }
        self->horizblur = m;

        m = ilG_material_new();
        ilG_material_vertex_file(m, "post.vert");
        ilG_material_name(m, "Vertical Blur Shader");
        ilG_material_arrayAttrib(m, ILG_ARRATTR_POSITION, "in_Position");
        ilG_material_arrayAttrib(m, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
        ilG_material_fragData(m, 0, "out_Color");
        ilG_material_bindFunc(m, size_uniform, self, "size");
        ilG_material_textureUnit(m, ILG_TUNIT_NONE, "tex");
        ilG_material_fragment_file(m, "vertblur.frag");
        if (ilG_material_link(m, context)) {
            return NULL;
        }
        self->vertblur = m;
    }
    
    m = ilG_material_new();
    ilG_material_vertex_file(m, "post.vert");
    ilG_material_name(m, "Tone Mapping Shader");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_bindFunc(m, size_uniform, self, "size");
    ilG_material_textureUnit(m, ILG_TUNIT_NONE, "tex");
    ilG_material_fragment_file(m, context->hdr? "hdr.frag" : "post.frag");
    if (ilG_material_link(m, context)) {
        return NULL;
    }
    self->material = m;

    static const float data[] = {
        0, 0,
	0, 1,
	1, 1,
	1, 0
    };
    glGenVertexArrays(1, &self->vao);
    glBindVertexArray(self->vao);
    glGenBuffers(1, &self->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
    glVertexAttribPointer(ILG_ARRATTR_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribPointer(ILG_ARRATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
    glEnableVertexAttribArray(ILG_ARRATTR_TEXCOORD);

    self->which = 1;

    return self;
}

