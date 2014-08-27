#include "context.h"
#include "context-internal.h"

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

#include "util/event.h"
#include "common/storage.h"
#include "graphics/graphics.h"
#include "graphics/transform.h"
#include "input/input.h"
#include "math/matrix.h"
#include "tgl/tgl.h"
#include "util/ilassert.h"
#include "util/log.h"
#include "util/logger.h"

#ifndef timeradd
# include "util/timer.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// Queue

void ilG_context_queue_init(ilG_context_queue *queue)
{
    queue->head = queue->tail = calloc(1, sizeof(struct ilG_context_msg));
    queue->first = (struct ilG_context_msg*)queue->head;
}

void ilG_context_queue_free(ilG_context_queue *queue)
{
    struct ilG_context_msg *cur;
    while (queue->first) {
        cur = queue->first;
        queue->first = cur->next;
        free(cur);
    }
}

// TODO: Multiple producer threads
void ilG_context_queue_produce(ilG_context_queue *queue, ilG_context_msg *msg)
{
    msg->next = NULL;
    queue->tail->next = msg;
    queue->tail = msg;
    struct ilG_context_msg *tmp;
    while (queue->first != queue->head) {
        tmp = queue->first;
        queue->first = queue->first->next;
        free(tmp);
    }
}

ilG_context_msg *ilG_context_queue_consume(ilG_context_queue *queue)
{
    if (queue->head != queue->tail) {
        return (struct ilG_context_msg*)queue->head->next;
    }
    return NULL;
}

void ilG_context_queue_doneconsume(ilG_context_queue *queue)
{
    if (queue->head != queue->tail)
    {
        queue->head = queue->head->next;
    }
}

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
    self->vsync = 1;
    self->initialTitle = "IntenseLogic";
    self->tick      = ilE_handler_new_with_name("il.graphics.context.tick");
    self->resize    = ilE_handler_new_with_name("il.graphics.context.resize");
    self->close     = ilE_handler_new_with_name("il.graphics.context.close");
    self->destroy   = ilE_handler_new_with_name("il.graphics.context.destroy");
    ilI_handler_init(&self->handler);
    tgl_fbo_init(&self->fb);
    self->queue = calloc(1, sizeof(struct ilG_context_queue));
    ilG_context_queue_init(self->queue);
    ilG_context_addRenderer(self, 0, ilG_builder_wrap(NULL, ilG_context_build));
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
    IL_APPEND(self->manager.objrenderers, obj);
    IL_APPEND(self->manager.viewrenderers, view);
    IL_APPEND(self->manager.statrenderers, stat);
    IL_APPEND(self->manager.coordsystems, co);
    self->root = (ilG_handle) {
        .id = 0,
        .context = self
    };
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
    default:
        il_error("Invalid hint");
    }
}

void ilG_context_free(ilG_context *self)
{
    assert(!self->running);
    self->complete = 0;

    il_value nil = il_value_nil();
    ilE_handler_fire(self->destroy, &nil);
    il_value_free(nil);

    for (unsigned i = 0; i < self->manager.renderers.length; i++) {
        ilG_renderer *r = &self->manager.renderers.data[i];
        r->free(r->data);
    }
    for (unsigned i = 0; i < self->manager.names.length; i++) {
        free(self->manager.names.data[i]);
    }
    for (unsigned i = 0; i < self->manager.coordsystems.length; i++) {
        ilG_coordsys *c = &self->manager.coordsystems.data[i];
        c->free(c->obj);
    }
    IL_FREE(self->manager.renderers);
    IL_FREE(self->manager.sinks);
    IL_FREE(self->manager.storages);
    IL_FREE(self->manager.namelinks);
    IL_FREE(self->manager.names);
    IL_FREE(self->manager.coordsystems);

    ilE_unregister(self->tick, self->tick_id);
    ilE_handler_destroy(self->tick);
    ilE_handler_destroy(self->resize);
    ilE_handler_destroy(self->close);
    ilG_context_queue_free(self->queue);

    SDL_GL_DeleteContext(self->context);
    SDL_DestroyWindow(self->window);
}

void ilG_context_bindFB(ilG_context *self)
{
    static const unsigned order[] = {
        ILG_CONTEXT_ACCUM,
        ILG_CONTEXT_NORMAL,
        ILG_CONTEXT_DIFFUSE,
        ILG_CONTEXT_SPECULAR
    };
    tgl_fbo_bind_with(&self->fb, TGL_FBO_WRITE, 4, order);
}

void ilG_context_bind_for_outpass(ilG_context *self)
{
    static const unsigned order[] = {
        ILG_CONTEXT_ACCUM
    };
    tgl_fbo_bind_with(&self->fb, TGL_FBO_READ, 1, order);
}

/////////////////////////////////////////////////////////////////////////////
// Message handlers

int ilG_context_localResize(ilG_context *self, int w, int h)
{
    self->width = w;
    self->height = h;

    il_value val;
    val = il_value_vectorl(2, il_value_int(self->width), il_value_int(self->height));
    ilE_handler_fire(self->resize, &val);
    il_value_free(val);

    if (self->use_default_fb) {
        self->valid = 1;
        return 1;
    }

    if (!tgl_fbo_build(&self->fb, w, h)) {
        return 0;
    }
    tgl_check("Error setting up screen");

    self->valid = 1;
    return 1;
}

static void context_message(ilG_context *self, ilG_rendid id, int type, il_value data)
{
    ilG_msgsink *s = ilG_context_findSink(self, id);
    ilG_renderer *r = ilG_context_findRenderer(self, id);
    assert(s && r);
    s->fn(r->data, type, &data);
    il_value_free(data);
}

/////////////////////////////////////////////////////////////////////////////
// Rendering logic and context setup

static void render_renderer(ilG_context *context, ilG_renderer *par)
{
    ilG_rendermanager *rm = &context->manager;
    ilG_renderer *r;
    for (unsigned i = 0, len = par->children.length; i < len; i++) {
        r = &rm->renderers.data[par->children.data[i]];
        ilG_rendid id = rm->rendids.data[par->children.data[i]];
        ilG_statrenderer *stat = &rm->statrenderers.data[r->stat];
        ilG_viewrenderer *view = &rm->viewrenderers.data[r->view];
        ilG_objrenderer *obj = &rm->objrenderers.data[r->obj];
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
        for (unsigned i = 0; i < obj->num_types; i++) {
            co->objmats(co->obj, obj->objects.data, num_mats, objmats[i], obj->types[i]);
            objmats_p[i] = &objmats[i][0];
        }
        obj->draw(r->data, id, objmats_p, obj->objects.data, num_mats);
        render_renderer(context, r);
    }
}

void ilG_context_renderFrame(ilG_context *context)
{
    glViewport(0, 0, context->width, context->height);

    il_debug("Begin render");
    ilG_context_bindFB(context);
    if (context->debug_render) {
        glClearColor(0.39, 0.58, 0.93, 1.0); // cornflower blue
    } else {
        glClearColor(0, 0, 0, 1.0);
    }
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    // asserts that first renderer is the context itself
    render_renderer(context, &context->manager.renderers.data[0]);
}

void ilG_context_measure(ilG_context *context)
{
    struct timeval time, tv;
    struct ilG_frame *iter, *temp, *frame, *last;

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

// TODO: Find out why APIENTRY disappeared when switching to SDL
static GLvoid error_cb(GLenum source, GLenum type, GLuint id, GLenum severity,
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

void ilG_context_setupSDLWindow(ilG_context *self) // main thread
{
    if (self->complete) {
        il_error("Context already complete");
        return;
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
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, self->msaa);
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
        return;
    }
    if (!(self->window = SDL_CreateWindow(
            self->initialTitle,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            self->startWidth,
            self->startHeight,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE))) {
        il_error("SDL_CreateWindow: %s", SDL_GetError());
        return;
    }
    self->width = self->startWidth;
    self->height = self->startHeight;
    SDL_SetWindowData(self->window, "context", self);
}

void ilG_context_setupGLEW(ilG_context *self)
{
    glewExperimental = self->experimental? GL_TRUE : GL_FALSE; // TODO: find out why IL crashes without this
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        il_error("glewInit() failed: %s", glewGetErrorString(err));
        return;
    }
    il_log("Using GLEW %s", glewGetString(GLEW_VERSION));

#ifndef __APPLE__
    if (!GLEW_VERSION_3_1) {
        il_error("GL version 3.1 is required, you have %s: crashes are on you", glGetString(GL_VERSION));
    } else {
        il_log("OpenGL Version %s", glGetString(GL_VERSION));
    }
#endif
    tgl_check("GLEW setup");
}

void ilG_context_localSetup(ilG_context *self)
{
    tgl_check("Unknown");
    if (GLEW_KHR_debug) {
        glDebugMessageCallback((GLDEBUGPROC)&error_cb, NULL);
        glEnable(GL_DEBUG_OUTPUT);
        il_log("KHR_debug present, enabling advanced errors");
        tgl_check("glDebugMessageCallback()");
    } else {
        il_log("KHR_debug missing");
    }
    if (!self->use_default_fb) {
        GLenum type = self->msaa? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_RECTANGLE;
        tgl_fbo_numTargets(&self->fb, ILG_CONTEXT_NUMATTACHMENTS);
        tgl_fbo_texture(&self->fb, ILG_CONTEXT_DEPTH, type, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);
        tgl_fbo_texture(&self->fb, ILG_CONTEXT_ACCUM, type, GL_RGBA8, GL_RGBA, GL_COLOR_ATTACHMENT0);
        tgl_fbo_texture(&self->fb, ILG_CONTEXT_NORMAL, type, GL_RGB8, GL_RGB, GL_COLOR_ATTACHMENT1);
        tgl_fbo_texture(&self->fb, ILG_CONTEXT_DIFFUSE, type, GL_RGB8, GL_RGB, GL_COLOR_ATTACHMENT2);
        tgl_fbo_texture(&self->fb, ILG_CONTEXT_SPECULAR, type, GL_RGBA8, GL_RGBA, GL_COLOR_ATTACHMENT3);
        if (self->msaa) {
            tgl_fbo_multisample(&self->fb, ILG_CONTEXT_DEPTH, self->msaa, false);
            tgl_fbo_multisample(&self->fb, ILG_CONTEXT_ACCUM, self->msaa, false);
            tgl_fbo_multisample(&self->fb, ILG_CONTEXT_NORMAL, self->msaa, false);
            tgl_fbo_multisample(&self->fb, ILG_CONTEXT_DIFFUSE, self->msaa, false);
            tgl_fbo_multisample(&self->fb, ILG_CONTEXT_SPECULAR, self->msaa, false);
        }
        tgl_check("Unable to generate framebuffer");
    }
    self->complete = 1;
}

void *ilG_context_loop(void *ptr)
{
    ilG_context *self = ptr;

    self->context = SDL_GL_CreateContext(self->window);

    SDL_GL_SetSwapInterval(self->vsync);
    ilG_context_setupGLEW(self);
    ilG_context_localSetup(self);
    ilG_context_localResize(self, self->startWidth, self->startHeight);

    while (self->valid) {
        // Run per-frame stuff
        il_value nil = il_value_nil();
        ilE_handler_fire(self->tick, &nil);
        il_value_free(nil);
        // Process messages
        struct ilG_context_msg *msg;
        while ((msg = ilG_context_queue_consume(self->queue))) {
            switch (msg->type) {
            case ILG_UPLOAD: msg->value.upload.cb(msg->value.upload.ptr); break;
            case ILG_RESIZE: ilG_context_localResize(self, msg->value.resize[0], msg->value.resize[1]); break;
            case ILG_STOP: goto stop;
            case ILG_MESSAGE: context_message(self, msg->value.message.node, msg->value.message.type, msg->value.message.data); break;
            case ILG_ADD_COORDS:
                ilG_context_addCoords(self, msg->value.coords.parent, msg->value.coords.cosys, msg->value.coords.codata);
                break;
            case ILG_DEL_COORDS:
                ilG_context_delCoords(self, msg->value.coords.parent, msg->value.coords.cosys, msg->value.coords.codata);
                break;
            case ILG_VIEW_COORDS:
                ilG_context_viewCoords(self, msg->value.coords.parent, msg->value.coords.cosys);
                break;
            case ILG_ADD_RENDERER:
                ilG_context_addChild(self, msg->value.renderer.parent, msg->value.renderer.child);
                break;
            case ILG_DEL_RENDERER:
                ilG_context_delChild(self, msg->value.renderer.parent, msg->value.renderer.child);
                break;
            case ILG_ADD_LIGHT:
                ilG_context_addLight(self, msg->value.light.parent, msg->value.light.child);
                break;
            case ILG_DEL_LIGHT:
                ilG_context_delLight(self, msg->value.light.parent, msg->value.light.child);
                break;
            }
            ilG_context_queue_doneconsume(self->queue);
        }
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
        // Perform fps measuring calculations
        ilG_context_measure(self);
    }

    if (!self->valid || !self->complete) {
        il_error("Tried to render invalid context");
    }

stop:
    ilG_context_queue_doneconsume(self->queue);
    tgl_fbo_free(&self->fb);
    return NULL;
}

bool ilG_context_start(ilG_context *self)
{
    ilG_context_setupSDLWindow(self);

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
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_STOP;
    ilG_context_queue_produce(self->queue, msg);
    pthread_join(self->thread, NULL);
    self->running = false;
}
