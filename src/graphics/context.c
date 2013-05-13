#include "context.h"

#include <stdlib.h>

#include "util/log.h"
#include "graphics/glutil.h"
#include "common/event.h"
#include "graphics/stage.h"
#include "graphics/graphics.h"

void context_cons(void *obj)
{
    ilG_context *self = obj;
    GLint num_texunits;

    ilG_testError("Unknown error before this function");
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &num_texunits);
    ilG_testError("glGetIntegerv");
    self->texunits = calloc(sizeof(unsigned), num_texunits);
    self->num_texunits = num_texunits;
    glGenFramebuffers(1, &self->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, self->framebuffer);
    glGenTextures(5, &self->fbtextures[0]);
    ilG_testError("Unable to generate framebuffer");
}

static void context_des(void *obj)
{
    ilG_context *self = obj;
    size_t i;

    self->complete = 0; // hopefully prevents use after free
    free(self->texunits);
    IL_FREE(self->positionables);
    for (i = 0; i < self->lights.length; i++) {
        il_unref(self->lights.data[i]);
    }
    IL_FREE(self->lights);
    for (i = 0; i < self->stages.length; i++) {
        il_unref(self->stages.data[i]);
    }
    IL_FREE(self->stages);
    glDeleteFramebuffers(1, &self->framebuffer);
    glDeleteTextures(5, &self->fbtextures[0]);
}

il_type ilG_context_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = context_cons,
    .destructor = context_des,
    .copy = NULL,
    .name = "il.graphics.context",
    .size = sizeof(ilG_context),
    .registry = NULL,
    .parent = NULL
};

void ilG_context_resize(ilG_context *self, int w, int h)
{
    self->width = w;
    self->height = h;
    ilG_testError("Unknown from before this function");
    glBindTexture(GL_TEXTURE_RECTANGLE, self->fbtextures[ILG_CONTEXT_DEPTH]); 
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, self->fbtextures[ILG_CONTEXT_DEPTH], 0);
    ilG_testError("Unable to create depth buffer");
    glBindTexture(GL_TEXTURE_RECTANGLE, self->fbtextures[ILG_CONTEXT_ACCUM]);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, self->fbtextures[ILG_CONTEXT_ACCUM], 0);
    ilG_testError("Unable to create accumulation buffer");
    glBindTexture(GL_TEXTURE_RECTANGLE, self->fbtextures[ILG_CONTEXT_NORMAL]); 
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, self->fbtextures[ILG_CONTEXT_NORMAL], 0);
    ilG_testError("Unable to create normal buffer");
    glBindTexture(GL_TEXTURE_RECTANGLE, self->fbtextures[ILG_CONTEXT_DIFFUSE]); 
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_RECTANGLE, self->fbtextures[ILG_CONTEXT_DIFFUSE], 0);
    ilG_testError("Unable to create diffuse buffer");
    glBindTexture(GL_TEXTURE_RECTANGLE, self->fbtextures[ILG_CONTEXT_SPECULAR]); 
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_RECTANGLE, self->fbtextures[ILG_CONTEXT_SPECULAR], 0);
    ilG_testError("Unable to create specular buffer");
    // completeness testing
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        const char *status_str;
        switch(status) {
            case GL_FRAMEBUFFER_UNDEFINED:                      status_str = "GL_FRAMEBUFFER_UNDEFINED";                        break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:          status_str = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";            break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:  status_str = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";    break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:         status_str = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";           break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:         status_str = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";           break;
            case GL_FRAMEBUFFER_UNSUPPORTED:                    status_str = "GL_FRAMEBUFFER_UNSUPPORTED";                      break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:         status_str = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";           break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:       status_str = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";         break;
            default:                                            status_str = "???";                                             break;
        }
        il_error("Unable to create framebuffer for context: %s", status_str);
        return;
    }
    self->complete = 1;
}

void ilG_context_addStage(ilG_context* self, ilG_stage* stage, int num)
{
    stage = il_ref(stage);
    if (num < 0) {
        IL_APPEND(self->stages, stage);
        return;
    }
    IL_INSERT(self->stages, (size_t)num, stage);
}

void render_stages(const ilE_registry* registry, const char *name, size_t size, const void *data, void * ctx)
{
    (void)registry, (void)name, (void)size, (void)data;
    ilG_context *context = ctx;
    size_t i;

    il_debug("Begin render");
    static const GLenum drawbufs[] = {
        GL_COLOR_ATTACHMENT0,   // accumulation
        GL_COLOR_ATTACHMENT1,   // normal
        GL_COLOR_ATTACHMENT2,   // diffuse
        GL_COLOR_ATTACHMENT3    // specular
    };
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, context->framebuffer);
    glDrawBuffers(4, &drawbufs[0]);
    glClearColor(1.0, 0.41, 0.72, 1.0); // hot pink
    glClear(GL_COLOR_BUFFER_BIT);
    for (i = 0; i < context->stages.length; i++) {
        il_debug("Rendering stage %s", context->stages.data[i]->name);
        context->stages.data[i]->run(context->stages.data[i]);
    }
}

void ilG_context_setActive(ilG_context *self)
{
    ilE_register(ilG_registry, "tick", ILE_DONTCARE, ILE_MAIN, render_stages, self);
}

