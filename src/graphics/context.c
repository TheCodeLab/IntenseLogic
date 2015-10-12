#include "context.h"
#include "context-internal.h"

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

#include "graphics/graphics.h"
#include "graphics/transform.h"
#include "math/matrix.h"
#include "tgl/tgl.h"
#include "util/log.h"
#include "util/logger.h"

#ifndef timeradd
# include "util/timer.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// Book keeping

ilG_context *ilG_context_new()
{
    ilG_context *self = malloc(sizeof(ilG_context));
    ilG_context_init(self);
    return self;
}

void ilG_context_init(ilG_context *self)
{
    memset(self, 0, sizeof(ilG_context));
    // hints
    self->contextMajor = 3;
#ifdef __APPLE__
    self->forwardCompat = 1;
#endif
    self->contextMinor = 2;
    self->profile = ILG_CONTEXT_NONE;
    self->experimental = 1;
    self->startWidth = 800;
    self->startHeight = 600;
    self->vsync = 1;
    self->initialTitle = "IntenseLogic";
    self->srgb = true;
    // public
    self->fovsquared = pow(45.f * M_PI / 180.f, 2.f);
    // private
    tgl_fbo_init(&self->gbuffer);
    tgl_fbo_init(&self->accum);
    ilG_renderman *rm = &self->manager;
    ilG_renderman_addRenderer(rm, 0, ilG_builder_wrap(NULL, ilG_context_build));
    ilG_statrenderer stat = (ilG_statrenderer) {ilG_default_update};
    ilG_viewrenderer view = (ilG_viewrenderer) {
        .update = ilG_default_multiupdate,
        .coordsys = 0,
        .types = NULL,
        .num_types = 0
    };
    ilG_objrenderer obj = (ilG_objrenderer) {
        .draw = ilG_default_draw,
        .coordsys = 0,
        .types = NULL,
        .num_types = 0
    };
    ilG_coordsys co = (ilG_coordsys) {
        .free = ilG_default_free,
        .viewmats = ilG_default_viewmats,
        .objmats = ilG_default_objmats,
        .obj = NULL,
        .id = 0,
    };
    IL_APPEND(rm->objrenderers, obj);
    IL_APPEND(rm->viewrenderers, view);
    IL_APPEND(rm->statrenderers, stat);
    IL_APPEND(rm->coordsystems, co);
}

void ilG_context_hint(ilG_context *self, enum ilG_context_hint hint, int param)
{
#define HINT(v, f) case v: self->f = param; break
    switch (hint) {
        HINT(ILG_CONTEXT_MAJOR, contextMajor);
        HINT(ILG_CONTEXT_MINOR, contextMinor);
        HINT(ILG_CONTEXT_FORWARD_COMPAT, forwardCompat);
        HINT(ILG_CONTEXT_PROFILE, profile);
        HINT(ILG_CONTEXT_DEBUG_CONTEXT, debug_context);
        HINT(ILG_CONTEXT_EXPERIMENTAL, experimental);
        HINT(ILG_CONTEXT_WIDTH, startWidth);
        HINT(ILG_CONTEXT_HEIGHT, startHeight);
        HINT(ILG_CONTEXT_HDR, hdr);
        HINT(ILG_CONTEXT_USE_DEFAULT_FB, use_default_fb);
        HINT(ILG_CONTEXT_DEBUG_RENDER, debug_render);
        HINT(ILG_CONTEXT_VSYNC, vsync);
        HINT(ILG_CONTEXT_MSAA, msaa);
        HINT(ILG_CONTEXT_SRGB, srgb);
    default:
        il_error("Invalid hint");
    }
}

void ilG_context_free(ilG_context *self)
{
    assert(!self->running);
    self->complete = 0;

    ilG_renderman_free(&self->manager);

    // private members
    tgl_fbo_free(&self->gbuffer);
    tgl_fbo_free(&self->accum);

    SDL_GL_DeleteContext(self->context);
    SDL_DestroyWindow(self->window);
}

/////////////////////////////////////////////////////////////////////////////
// Message handlers

int ilG_context_localResize(ilG_context *self, int w, int h)
{
    self->width = w;
    self->height = h;

    if (self->use_default_fb) {
        self->valid = 1;
        return 1;
    }

    if (!tgl_fbo_build(&self->gbuffer, w, h) || !tgl_fbo_build(&self->accum, w, h)) {
        return 0;
    }
    tgl_check("Error setting up screen");

    self->valid = 1;
    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// Rendering logic and context setup

static void render_renderer(ilG_context *context, ilG_renderer *par)
{
    ilG_renderman *rm = &context->manager;
    const bool have_debug = context->have_khr_debug;
    for (unsigned i = 0, len = par->children.length; i < len; i++) {
        ilG_renderer *r = &rm->renderers.data[par->children.data[i]];
        ilG_rendid id = rm->rendids.data[par->children.data[i]];
        ilG_statrenderer *stat = &rm->statrenderers.data[r->stat];
        ilG_viewrenderer *view = &rm->viewrenderers.data[r->view];
        ilG_objrenderer *obj = &rm->objrenderers.data[r->obj];
        if (have_debug) {
            const char *name = ilG_renderman_findName(rm, id);
            glPushDebugGroup(GL_DEBUG_SOURCE_THIRD_PARTY, 0, -1, name? name : "Unknown Renderer");
        }
        stat->update(r->data, id);
        tgl_check("In %s static update", ilG_context_findName(context, id));

        ilG_coordsys *co = &rm->coordsystems.data[view->coordsys];
        il_mat viewmats[view->num_types];
        co->viewmats(co->obj, viewmats, view->types, view->num_types);
        view->update(r->data, id, viewmats);
        tgl_check("In %s view update", ilG_context_findName(context, id));

        co = &rm->coordsystems.data[obj->coordsys];
        unsigned num_mats = obj->objects.length;
        il_mat objmats[obj->num_types][num_mats];
        il_mat *objmats_p[obj->num_types];
        for (unsigned i = 0; i < obj->num_types && num_mats; i++) {
            co->objmats(co->obj, obj->objects.data, num_mats, objmats[i], obj->types[i]);
            objmats_p[i] = &objmats[i][0];
        }
        obj->draw(r->data, id, objmats_p, obj->objects.data, num_mats);
        render_renderer(context, r);
        if (have_debug) {
            glPopDebugGroup();
        }
    }
}

void ilG_context_renderFrame(ilG_context *context)
{
    glViewport(0, 0, context->width, context->height);

    il_debug("Begin render");
    tgl_fbo_bind(&context->accum, TGL_FBO_WRITE);
    glClearColor(0,0,0, 1.0);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_FRAMEBUFFER_SRGB);

    // asserts that first renderer is the context itself
    render_renderer(context, &context->manager.renderers.data[0]);
}

typedef struct {
    IL_ARRAY(char*,) groups;
} ilG_group_stack;

// TODO: Find out why APIENTRY disappeared when switching to SDL
static GLvoid error_cb(GLenum source, GLenum type, GLuint id, GLenum severity,
                       GLsizei length, const GLchar* message, GLvoid* user)
{
    ilG_group_stack *stack = user;
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
        case GL_DEBUG_TYPE_PUSH_GROUP: {
            IL_APPEND(stack->groups, strdup(message));
            return;
        }
        case GL_DEBUG_TYPE_POP_GROUP: {
            assert(stack->groups.length > 0);
            free(stack->groups.data[--stack->groups.length]);
            return;
        }
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
    size_t source_len = snprintf(source_buf, 64, "OpenGL%s", ssource);

    char group_buf[4096] = "";
    if (stack->groups.length) {
        strcat(group_buf, " in ");
    }
    for (unsigned i = 0; i < stack->groups.length; i++) {
        if (i > 0) {
            strcat(group_buf, ".");
        }
        strcat(group_buf, stack->groups.data[i]);
    }

    const char *msg_fmt = "%s%s #%u%s: %s";
    size_t len = snprintf(NULL, 0, msg_fmt, sseverity, stype, id, group_buf, msg);
    char msg_buf[len+1];
    snprintf(msg_buf, len+1, msg_fmt, sseverity, stype, id, group_buf, msg);

    il_logmsg lmsg;
    memset(&lmsg, 0, sizeof(il_logmsg));
    lmsg.level = IL_NOTIFY;
    lmsg.msg = il_string_bin(msg_buf, len);
    lmsg.func = il_string_bin(source_buf, source_len);

    il_logger *logger = il_logger_cur();
    il_logger_log(logger, lmsg);
}

extern bool ilG_module_loaded;

bool ilG_context_setupSDLWindow(ilG_context *self) // main thread
{
    if (!ilG_module_loaded) {
        il_error("il_load_ilgraphics() has not been called");
        abort();
    }
    if (self->complete) {
        il_error("Context already complete");
        return false;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, self->contextMajor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, self->contextMinor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
        (self->forwardCompat? SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG : 0) |
        (self->debug_context? SDL_GL_CONTEXT_DEBUG_FLAG : 0)
    );
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, self->msaa != 0);
    if (self->msaa) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, self->msaa);
    }
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, self->srgb);
    switch (self->profile) {
        case ILG_CONTEXT_NONE:
        break;
        case ILG_CONTEXT_CORE:
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        break;
        case ILG_CONTEXT_COMPAT:
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        break;
        default:
        il_error("Invalid profile");
        return false;
    }
    if (!(self->window = SDL_CreateWindow(
            self->initialTitle,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            self->startWidth,
            self->startHeight,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE))) {
        il_error("SDL_CreateWindow: %s", SDL_GetError());
        return false;
    }
    self->width = self->startWidth;
    self->height = self->startHeight;
    SDL_SetWindowData(self->window, "context", self);
    return true;
}

#ifdef TGL_USE_GLEW
void ilG_context_setupGLEW(ilG_context *self)
{
    glewExperimental = self->experimental? GL_TRUE : GL_FALSE; // TODO: find out why IL crashes without this
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        il_error("glewInit() failed: %s", glewGetErrorString(err));
        return;
    }
    il_log("Using GLEW %s", glewGetString(GLEW_VERSION));

# ifdef __APPLE__
    if (!GLEW_VERSION_3_2) {
# else
    if (!GLEW_VERSION_3_1) {
# endif
        il_error("GL version 3.2 is required, you have %s: crashes are on you", glGetString(GL_VERSION));
    } else {
        il_log("OpenGL Version %s", glGetString(GL_VERSION));
    }
    tgl_check("GLEW setup");
}
#endif

#ifdef TGL_USE_EPOXY
void ilG_context_setupEpoxy(ilG_context *self)
{
    (void)self;
    if (epoxy_gl_version() < 31) {
        il_error("GL version 3.1 is required: a crash is likely");
    }
    il_log("OpenGL Version %s", glGetString(GL_VERSION));
}
#endif

void ilG_context_localSetup(ilG_context *self)
{
    tgl_check("Unknown");
    if (TGL_EXTENSION(KHR_debug)) {
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PUSH_GROUP, GL_DONT_CARE, 0, NULL, true);
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_POP_GROUP, GL_DONT_CARE, 0, NULL, true);
        glDebugMessageCallback((GLDEBUGPROC)&error_cb, calloc(1, sizeof(ilG_group_stack)));
        glEnable(GL_DEBUG_OUTPUT);
        il_log("KHR_debug present, enabling advanced errors");
        tgl_check("glDebugMessageCallback()");
        self->have_khr_debug = true;
    } else {
        il_log("KHR_debug missing");
    }
    if (!self->use_default_fb) {
        GLenum type = self->msaa? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_RECTANGLE;
        GLenum afmt = self->hdr? GL_RGBA16F : GL_RGBA8;
        tgl_fbo_numTargets(&self->gbuffer, ILG_CONTEXT_NUMATTACHMENTS);
        // Per GL 3.1 spec §4.2 (page 182, actual 195):
        // Each COLOR_ATTACHMENTi adheres to COLOR_ATTACHMENTi = COLOR_ATTACHMENT0 + i.
        tgl_fbo_texture(&self->gbuffer, ILG_CONTEXT_ALBEDO, type, GL_RGB8, GL_RGB,
                        GL_COLOR_ATTACHMENT0 + ILG_CONTEXT_ALBEDO, GL_UNSIGNED_BYTE);
        tgl_fbo_texture(&self->gbuffer, ILG_CONTEXT_NORMAL, type, GL_RGB8_SNORM, GL_RGB,
                        GL_COLOR_ATTACHMENT0 + ILG_CONTEXT_NORMAL, GL_UNSIGNED_BYTE);
        tgl_fbo_texture(&self->gbuffer, ILG_CONTEXT_REFRACTION, type, GL_R8, GL_RED,
                        GL_COLOR_ATTACHMENT0 + ILG_CONTEXT_REFRACTION, GL_UNSIGNED_BYTE);
        tgl_fbo_texture(&self->gbuffer, ILG_CONTEXT_GLOSS, type, GL_R16F, GL_RED,
                        GL_COLOR_ATTACHMENT0 + ILG_CONTEXT_GLOSS, GL_FLOAT);
        tgl_fbo_texture(&self->gbuffer, ILG_CONTEXT_EMISSION, type, GL_R16F, GL_RED,
                        GL_COLOR_ATTACHMENT0 + ILG_CONTEXT_EMISSION, GL_FLOAT);
        tgl_fbo_texture(&self->gbuffer, ILG_CONTEXT_DEPTH, type, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT, GL_FLOAT);
        tgl_fbo_numTargets(&self->accum, 1);
        tgl_fbo_texture(&self->accum, 0, type, afmt, GL_RGBA, GL_COLOR_ATTACHMENT0, self->hdr? GL_FLOAT : GL_UNSIGNED_BYTE);
        if (self->msaa) {
            tgl_fbo_multisample(&self->gbuffer, ILG_CONTEXT_DEPTH, self->msaa, false);
            tgl_fbo_multisample(&self->gbuffer, ILG_CONTEXT_NORMAL, self->msaa, false);
            tgl_fbo_multisample(&self->gbuffer, ILG_CONTEXT_ALBEDO, self->msaa, false);
            tgl_fbo_multisample(&self->gbuffer, ILG_CONTEXT_REFRACTION, self->msaa, false);
            tgl_fbo_multisample(&self->gbuffer, ILG_CONTEXT_GLOSS, self->msaa, false);
            tgl_fbo_multisample(&self->gbuffer, ILG_CONTEXT_EMISSION, self->msaa, false);
            tgl_fbo_multisample(&self->accum, 0, self->msaa, false);
        }
        tgl_check("Unable to generate framebuffer");
    }
    glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
    self->complete = 1;
}

void *ilG_context_loop(void *ptr)
{
    ilG_context *self = ptr;

    self->context = SDL_GL_CreateContext(self->window);

    SDL_GL_SetSwapInterval(self->vsync);
#ifdef TGL_USE_GLEW
    ilG_context_setupGLEW(self);
#elif defined(TGL_USE_EPOXY)
    ilG_context_setupEpoxy(self);
#endif
    ilG_context_localSetup(self);
    ilG_context_localResize(self, self->startWidth, self->startHeight);

    if (!self->valid || !self->complete) {
        il_error("Tried to render invalid context");
        return NULL;
    }

    while (self->running) {
        int width, height;
        SDL_GetWindowSize(self->window, &width, &height);
        if (width != self->width || height != self->height) {
            ilG_context_localResize(self, width, height);
        }
        // Render
        ilG_context_renderFrame(self);
        if (self->use_default_fb) {
            glViewport(0,0, self->width, self->height);
            SDL_GL_SwapWindow(self->window);
        }
    }

    return NULL;
}

bool ilG_context_start(ilG_context *self)
{
    if (!ilG_context_setupSDLWindow(self)) {
        return false;
    }

    self->running = true;
    // Start thread
    int res = pthread_create(&self->thread, NULL, ilG_context_loop, self);
    if (res) {
        il_error("pthread_create: %s", strerror(errno));
    }
    return res == 0;
}

void ilG_context_stop(ilG_context *self)
{
    self->running = false;
    pthread_join(self->thread, NULL);
}

void ilG_context_print(ilG_context *self)
{
#define log(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)
    log("resolution: %i x %i", self->width, self->height);
    log("title: %s", self->title);
    log("gl version: %i.%i", self->contextMajor, self->contextMinor);
#define flag(n) self->n? "+" #n : "-" #n
    log("%s %s %s", flag(valid), flag(running), flag(complete));
    log("flags: %s %s %s %s %s %s %s %s",
        flag(forwardCompat), flag(debug_context), flag(experimental), flag(hdr),
        flag(use_default_fb), flag(debug_render), flag(vsync), flag(msaa));
#undef flag
    ilG_renderman_print(self, 0);
}
