#include "context.h"

#include <string.h>

#include "util/log.h"
#include "util/logger.h"
#include "util/ilassert.h"
#include "util/timer.h"
#include "graphics/glutil.h"
#include "common/event.h"
#include "graphics/graphics.h"
#include "input/input.h"

ilG_context *ilG_context_new()
{
    ilG_context *self = calloc(1, sizeof(ilG_context));
    self->contextMajor = 3;
#ifdef __APPLE__
    self->contextMinor = 2;
    self->forwardCompat = 1;
#else
    self->contextMinor = 1;
#endif
    self->profile = ILG_CONTEXT_NONE;
    self->experimental = 1;
    self->startWidth = 800;
    self->startHeight = 600;
    self->initialTitle = "IntenseLogic";
    self->resize    = ilE_handler_new_with_name("il.graphics.context.resize");
    self->close     = ilE_handler_new_with_name("il.graphics.context.close");
    self->destroy   = ilE_handler_new_with_name("il.graphics.context.destroy");
    ilI_handler_init(&self->handler);
    return self;
}

void ilG_context_free(ilG_context *self)
{
    ilG_context_renderer.free(self);
}

void ilG_context_hint(ilG_context *self, enum ilG_context_hint hint, int param)
{
#define HINT(v, f) case v: self->f = param; break;
    switch (hint) {
        HINT(ILG_CONTEXT_MAJOR, contextMajor)
        HINT(ILG_CONTEXT_MINOR, contextMinor)
        HINT(ILG_CONTEXT_FORWARD_COMPAT, forwardCompat)
        HINT(ILG_CONTEXT_PROFILE, profile)
        HINT(ILG_CONTEXT_DEBUG_CONTEXT, debug_context)
        HINT(ILG_CONTEXT_EXPERIMENTAL, experimental)
        HINT(ILG_CONTEXT_WIDTH, startWidth)
        HINT(ILG_CONTEXT_HEIGHT, startHeight)
        HINT(ILG_CONTEXT_HDR, hdr)
        HINT(ILG_CONTEXT_USE_DEFAULT_FB, use_default_fb)
        HINT(ILG_CONTEXT_DEBUG_RENDER, debug_render)
        default:
        il_error("Invalid hint");
    }
}

int ilG_context_build(ilG_context *self)
{
    return ilG_context_renderer.build(self, NULL);
}

int ilG_context_resize(ilG_context *self, int w, int h, const char *title)
{
    if (!self->complete) {
        il_error("Resizing incomplete context");
        return 0;
    }

    // GL setup
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    IL_GRAPHICS_TESTERROR("Error setting up screen");

    self->width = w;
    self->height = h;
    if (title != self->title && title) {
        if (self->title) {
            free(self->title);
        }
        self->title = strdup(title);
        glfwSetWindowTitle(self->window, self->title);
        if (self->use_default_fb) {
            self->valid = 1;
            return 1;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, self->framebuffer);
    glBindTexture(GL_TEXTURE_RECTANGLE, self->fbtextures[ILG_CONTEXT_DEPTH]);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, self->fbtextures[ILG_CONTEXT_DEPTH], 0);
    ilG_testError("Unable to create depth buffer");
    glBindTexture(GL_TEXTURE_RECTANGLE, self->fbtextures[ILG_CONTEXT_ACCUM]);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, w, h, 0, GL_RGBA, self->hdr? GL_FLOAT : GL_UNSIGNED_BYTE, NULL);
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
        return 0;
    }
    il_value val = il_value_vectorl(2, il_value_int(self->width), il_value_int(self->height));
    ilE_handler_fire(self->resize, &val);
    il_value_free(val);
    self->valid = 1;
    return 1;
}

void ilG_context_makeCurrent(ilG_context *self)
{
    glfwMakeContextCurrent(self->window);
}

void ilG_context_bindFB(ilG_context *self)
{
    static const GLenum drawbufs[] = {
        GL_COLOR_ATTACHMENT0,   // accumulation
        GL_COLOR_ATTACHMENT1,   // normal
        GL_COLOR_ATTACHMENT2,   // diffuse
        GL_COLOR_ATTACHMENT3    // specular
    };
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, self->framebuffer);
    glDrawBuffers(4, &drawbufs[0]);
}

void ilG_context_bind_for_outpass(ilG_context *self)
{
    /*static const GLenum drawbufs[] = {
        GL_COLOR_ATTACHMENT0    // accumulation
    };*/
    glBindFramebuffer(GL_READ_FRAMEBUFFER, self->framebuffer);
    //glDrawBuffers(1, &drawbufs[0]);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
}

static void render_stages(const il_value *data, il_value *ctx)
{
    (void)data;
    ilG_context *context = il_value_tomvoid(ctx);
    int width, height;
    struct timeval time, tv;
    struct ilG_frame *iter, *temp, *frame, *last;

    glfwPollEvents();

    if (!context->complete || !context->valid) {
        il_error("Rendering invalid context");
        return;
    }

    ilG_context_makeCurrent(context);
    glfwGetFramebufferSize(context->window, &width, &height);
    if (width != context->width || height != context->height) {
        ilG_context_resize(context, width, height, context->title);
    }
    glViewport(0, 0, width, height);
    context->material       = NULL;
    context->materialb      = NULL;
    context->drawable       = NULL;
    context->drawableb      = NULL;
    context->texture        = NULL;
    context->textureb       = NULL;
    context->positionable   = NULL;

    il_debug("Begin render");
    if (context->use_default_fb) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        ilG_context_bindFB(context);
    }
    if (context->debug_render) {
        glClearColor(0.39, 0.58, 0.93, 1.0); // cornflower blue
    } else {
        glClearColor(0, 0, 0, 1.0);
    }
    ilG_testError("glClearColor");
    glClearDepth(1.0);
    ilG_testError("glClearDepth");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ilG_context_renderer.draw(context);

    if (context->use_default_fb) {
        glfwSwapBuffers(context->window);
    }
    gettimeofday(&time, NULL);
    IL_LIST_ITER(context->frames_head, ll, iter, temp) {
        timersub(&time, &iter->start, &tv);
        if (tv.tv_sec > 0) {
            IL_LIST_POPHEAD(context->frames_head, ll, frame);
            timersub(&context->frames_sum, &frame->elapsed, &context->frames_sum);
            context->num_frames--;
            free(frame);
        }
    }
    last = IL_LIST_TAIL(context->frames_head, ll);
    frame = calloc(1, sizeof(struct ilG_frame));
    frame->start = time;
    if (last) {
        timersub(&time, &last->start, &frame->elapsed);
    } else {
        frame->elapsed = (struct timeval){0,0};
    }
    IL_LIST_APPEND(context->frames_head, ll, frame);
    context->num_frames++;
    timeradd(&frame->elapsed, &context->frames_sum, &context->frames_sum);
    context->frames_average.tv_sec = context->frames_sum.tv_sec / context->num_frames;
    context->frames_average.tv_usec = context->frames_sum.tv_usec / context->num_frames;
    context->frames_average.tv_usec += (context->frames_sum.tv_sec % context->num_frames) * 1000000 / context->num_frames;
}

int ilG_context_start(ilG_context *self)
{
    if (!self->complete) {
        il_error("Incomplete context");
        return 0;
    }
    if (!self->valid) {
        il_error("Invalid context");
        return 0;
    }
    if (!self->camera) {
        il_error("No camera");
        return 0;
    }
    if (!self->world) {
        il_error("No world");
        return 0;
    }
    self->tick_id = ilE_register(self->tick, ILE_BEFORE, ILE_MAIN, render_stages, il_value_opaque(il_opaque(self, NULL)));
    return 1;
}


