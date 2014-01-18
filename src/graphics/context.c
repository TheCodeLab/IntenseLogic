#include "context.h"

#include <string.h>

#include "util/log.h"
#include "util/logger.h"
#include "util/ilassert.h"
#include "util/timer.h"
#include "graphics/glutil.h"
#include "common/event.h"
#include "graphics/stage.h"
#include "graphics/graphics.h"
#include "input/input.h"

static GLvoid error_cb(GLenum source, GLenum type, GLuint id, GLenum severity,
                       GLsizei length, const GLchar* message, GLvoid* user);
void ilG_registerInputBackend(ilG_context *ctx);

static void on_close(const ilE_handler *handler, size_t size, const void *data, void *ctx);
static void on_close2(const ilE_handler *handler, size_t size, const void *data, void *ctx);

static void context_cons(void *obj)
{
    ilG_context *self = obj;
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
    self->close_id  = ilE_register(self->close, ILE_BEFORE, ILE_ANY, &on_close, self);
    self->close2_id = ilE_register(self->close, ILE_AFTER, ILE_ANY, &on_close2, self);
    ilI_handler_init(&self->input_handler);
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
    ilE_unregister(self->close, self->close_id);
    ilE_unregister(self->close, self->close2_id);
    ilE_handler_destroy(self->close);
}

il_type ilG_context_type = {
    .typeclasses = NULL,
    .storage = {NULL},
    .constructor = context_cons,
    .destructor = context_des,
    .copy = NULL,
    .name = "il.graphics.context",
    .size = sizeof(ilG_context),
    .parent = NULL
};

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
    if (self->complete) {
        il_error("Context already complete");
        return 0;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, self->contextMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, self->contextMinor);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, self->forwardCompat? GL_TRUE : GL_FALSE);
    switch (self->profile) {
        case ILG_CONTEXT_NONE:
        break;
        case ILG_CONTEXT_CORE:
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        break;
        case ILG_CONTEXT_COMPAT:
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
        break;
        default:
        il_error("Invalid profile");
        return 0;
    }
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, self->debug_context? GL_TRUE : GL_FALSE);
    if (!(self->window = glfwCreateWindow(self->startWidth, self->startHeight, self->initialTitle, NULL, NULL))) { // TODO: allow context sharing + monitor specification
        il_error("glfwOpenWindow() failed - are you sure you have OpenGL 3.1?");
        return 0;
    }
    ilG_registerInputBackend(self);
    glfwSetWindowUserPointer(self->window, self);
    ilG_context_makeCurrent(self);
    glfwSwapInterval(0);
    glewExperimental = self->experimental? GL_TRUE : GL_FALSE; // TODO: find out why IL crashes without this
    struct timeval tv;
    tv.tv_sec = 0;
    GLFWmonitor *mon = glfwGetWindowMonitor(self->window); // NULL if windowed mode
    if (mon) {
        const GLFWvidmode* mode = glfwGetVideoMode(mon);
        tv.tv_usec = 1000000 / mode->refreshRate;
    } else {
        tv.tv_usec = 1000000 / 250; // TODO: Unlimited framerates, proper vsync
    }
    self->tick = ilE_handler_timer(&tv);
    ilE_handler_name(self->tick, "il.graphics.context.tick");
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        il_error("glewInit() failed: %s", glewGetErrorString(err));
        return 0;
    }
    il_log("Using GLEW %s", glewGetString(GLEW_VERSION));

#ifndef __APPLE__
    if (!GLEW_VERSION_3_1) {
        il_error("GL version 3.1 is required, you have %s: crashes are on you", glGetString(GL_VERSION));
    } else {
        il_log("OpenGL Version %s", glGetString(GL_VERSION));
    }
#endif

    IL_GRAPHICS_TESTERROR("Unknown");
    if (GLEW_KHR_debug) {
        glDebugMessageCallback((GLDEBUGPROC)&error_cb, NULL);
        glEnable(GL_DEBUG_OUTPUT);
        il_log("KHR_debug present, enabling advanced errors");
        IL_GRAPHICS_TESTERROR("glDebugMessageCallback()");
    } else {
        il_log("KHR_debug missing");
    }
    GLint num_texunits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &num_texunits);
    ilG_testError("glGetIntegerv");
    self->texunits = calloc(sizeof(unsigned), num_texunits);
    self->num_texunits = num_texunits;
    if (self->use_default_fb) {
        self->complete = 1;
        return 1;
    }
    glGenFramebuffers(1, &self->framebuffer);
    glGenTextures(5, &self->fbtextures[0]);
    ilG_testError("Unable to generate framebuffer");
    self->complete = 1;
    return 1;
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
    ilE_handler_fire(self->resize, sizeof(ilG_context), self);
    self->valid = 1;
    return 1;
}

void ilG_context_makeCurrent(ilG_context *self)
{
    glfwMakeContextCurrent(self->window);
}

void ilG_context_addStage(ilG_context* self, ilG_stage* stage, int num)
{
    il_return_on_fail(stage);
    stage = il_ref(stage);
    if (num < 0) {
        IL_APPEND(self->stages, stage);
        return;
    }
    IL_INSERT(self->stages, (size_t)num, stage);
}

void ilG_context_clearStages(ilG_context *self)
{
    self->stages.length = 0;
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

static void render_stages(const ilE_handler* registry, size_t size, const void *data, void * ctx)
{
    (void)registry, (void)size, (void)data;
    ilG_context *context = ctx;
    size_t i;
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
    for (i = 0; i < context->stages.length; i++) {
        il_debug("Rendering stage %s", context->stages.data[i]->name);
        context->stages.data[i]->run(context->stages.data[i]);
    }
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

static void on_close(const ilE_handler *handler, size_t size, const void *data, void *ctx)
{
    (void)handler; (void)size; (void)data;
    ilG_context *self = ctx;
    ilE_unregister(self->tick, self->tick_id);
}

static void on_close2(const ilE_handler *handler, size_t size, const void *data, void *ctx)
{
    (void)handler, (void)size, (void)data;
    ilG_context *self = ctx;
    ilE_handler_destroy(self->tick);
    ilE_handler_destroy(self->resize);
    //il_unref(self);
}

int ilG_context_setActive(ilG_context *self)
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
    self->tick_id = ilE_register(self->tick, ILE_BEFORE, ILE_MAIN, render_stages, self);
    return 1;
}

static APIENTRY GLvoid error_cb(GLenum source, GLenum type, GLuint id, GLenum severity,
                       GLsizei length, const GLchar* message, GLvoid* user)
{
    (void)user;
    const char *ssource;
    switch(source) {
        case GL_DEBUG_SOURCE_API_ARB:               ssource=" API";              break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:     ssource=" Window System";    break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:   ssource=" Shader Compiler";  break;
        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:       ssource=" Third Party";      break;
        case GL_DEBUG_SOURCE_APPLICATION_ARB:       ssource=" Application";      break;
        case GL_DEBUG_SOURCE_OTHER_ARB:             ssource="";            break;
        default: ssource="???";
    }
    const char *stype;
    switch(type) {
        case GL_DEBUG_TYPE_ERROR_ARB:               stype=" error";                 break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: stype=" deprecated behaviour";  break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:  stype=" undefined behaviour";   break;
        case GL_DEBUG_TYPE_PORTABILITY_ARB:         stype=" portability issue";     break;
        case GL_DEBUG_TYPE_PERFORMANCE_ARB:         stype=" performance issue";     break;
        case GL_DEBUG_TYPE_OTHER_ARB:               stype="";                       break;
        default: stype="???";
    }
    const char *sseverity;
    switch(severity) {
        case GL_DEBUG_SEVERITY_HIGH_ARB:    sseverity="high";   break;
        case GL_DEBUG_SEVERITY_MEDIUM_ARB:  sseverity="medium"; break;
        case GL_DEBUG_SEVERITY_LOW_ARB:     sseverity="low";    break;
        default: sseverity="???";
    }
    char msg[length+1];
    strncpy(msg, message, length+1);
    if (msg[length-1] == '\n') {
        msg[length-1] = 0; // cut off newline
    }

    char source_buf[64];
    snprintf(source_buf, 64, "OpenGL%s", ssource);

    const char *msg_fmt = "%s%s #%u: %s";
    size_t len = snprintf(NULL, 0, msg_fmt, sseverity, stype, id, msg);
    char msg_buf[len+1];
    snprintf(msg_buf, len+1, msg_fmt, sseverity, stype, id, msg);

    il_logmsg *lmsg = il_logmsg_new(1);
    il_logmsg_setLevel(lmsg, IL_NOTIFY);
    il_logmsg_copyMessage(lmsg, msg_buf);
    il_logmsg_copyBtString(lmsg, 0, source_buf);

    il_logger *logger = il_logger_stderr; // TODO
    il_logger_log(logger, lmsg);
}

