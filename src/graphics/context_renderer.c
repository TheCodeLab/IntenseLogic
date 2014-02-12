#include "graphics/context.h"

#include "util/log.h"
#include "util/logger.h"
#include "input/input.h"
#include "graphics/glutil.h"

static void context_free(void *ptr) 
{
    ilG_context *self = ptr;
    self->complete = 0;

    il_value nil = il_value_nil();
    ilE_handler_fire(self->destroy, &nil);
    il_value_free(nil);

    free(self->texunits);
    unsigned i;
    for (i = 0; i < self->renderers.length; i++) {
        ilG_renderer_free(self->renderers.data[i]);
    }
    IL_FREE(self->renderers);
    glDeleteFramebuffers(1, &self->framebuffer);
    glDeleteTextures(5, &self->fbtextures[0]);
    ilE_unregister(self->tick, self->tick_id);
    ilE_handler_destroy(self->tick);
    ilE_handler_destroy(self->resize);
    ilE_handler_destroy(self->close);

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

static void context_draw(void *ptr)
{
    ilG_context *self = ptr;

    for (unsigned i = 0; i < self->renderers.length; i++) {
        il_debug("Rendering %s", ilG_renderer_getName(&self->renderers.data[i]));
        self->renderers.data[i].vtable->draw(self->renderers.data[i].obj);
    }
}

void ilG_registerInputBackend(ilG_context *ctx);
static int context_build(void *ptr, ilG_context *ctx)
{
    (void)ctx;
    ilG_context *self = ptr;
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

static il_table *context_get_storage(void *ptr)
{
    ilG_context *self = ptr;
    return &self->storage;
}

static bool context_get_complete(const void *ptr)
{
    const ilG_context *self = ptr;
    return self->complete;
}

static void context_add_renderer(void *ptr, ilG_renderer r)
{
    ilG_context *self = ptr;
    IL_APPEND(self->renderers, r);
}

const ilG_renderable ilG_context_renderer = {
    .free = context_free,
    .draw = context_draw,
    .build = context_build,
    .get_storage = context_get_storage,
    .get_complete = context_get_complete,
    .add_positionable = NULL,
    .add_renderer = context_add_renderer
};

