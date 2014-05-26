#include "context.h"

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

#include "util/log.h"
#include "util/logger.h"
#include "util/ilassert.h"
#ifndef timeradd
# include "util/timer.h"
#endif
#include "graphics/glutil.h"
#include "common/event.h"
#include "graphics/graphics.h"
#include "input/input.h"

/////////////////////////////////////////////////////////////////////////////
// Queue 

// TODO: Generalize this and split into its own file
struct ilG_context_msg {
    struct ilG_context_msg *next;
    enum ilG_context_msgtype {
        ILG_UPLOAD,
        ILG_RESIZE,
        ILG_STOP,
        ILG_MESSAGE,
        ILG_ADD_RENDERER,
        ILG_DEL_RENDERER,
        ILG_ADD_LIGHT,
        ILG_DEL_LIGHT
    } type;
    union {
        struct {
            void (*cb)(void*);
            void *ptr;
        } upload;
        int resize[2];
        struct {
            ilG_rendid node;
            int type;
            il_value data;
        } message;
        struct {
            ilG_rendid parent, child;
        } renderer;
        struct {
            ilG_rendid parent;
            il_positionable child;
        } positionable;
        struct {
            ilG_rendid parent;
            ilG_light child;
        } light;
    } value;
};

struct ilG_context_queue {
    struct ilG_context_msg *first;
    volatile struct ilG_context_msg *head, *tail;
};

static void queue_init(struct ilG_context_queue *queue)
{
    queue->head = queue->tail = calloc(1, sizeof(struct ilG_context_msg));
    queue->first = (struct ilG_context_msg*)queue->head;
}

void queue_free(struct ilG_context_queue *queue) // TODO: Fix this cross-translation unit hack
{
    struct ilG_context_msg *cur;
    while (queue->first) {
        cur = queue->first;
        queue->first = cur->next;
        free(cur);
    }
}

// TODO: Multiple producer threads
static void produce(struct ilG_context_queue *queue, struct ilG_context_msg *msg)
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

static struct ilG_context_msg *consume(struct ilG_context_queue *queue)
{
    if (queue->head != queue->tail) {
        return (struct ilG_context_msg*)queue->head->next;
    }
    return NULL;
}

static void done_consume(struct ilG_context_queue *queue)
{
    if (queue->head != queue->tail)
    {
        queue->head = queue->head->next;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Book keeping

void ilG_registerInputBackend(ilG_context *ctx);

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
    self->tick      = ilE_handler_new_with_name("il.graphics.context.tick");
    self->resize    = ilE_handler_new_with_name("il.graphics.context.resize");
    self->close     = ilE_handler_new_with_name("il.graphics.context.close");
    self->destroy   = ilE_handler_new_with_name("il.graphics.context.destroy");
    ilI_handler_init(&self->handler);
    self->queue = calloc(1, sizeof(struct ilG_context_queue));
    queue_init(self->queue);
    ilG_context_addRenderer(self, 0, ilG_builder_wrap(NULL, ilG_context_build));
    self->root = (ilG_handle) {
        .id = 0,
        .context = self
    };
    return self;
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
        HINT(ILG_CONTEXT_VSYNC, vsync);
        default:
        il_error("Invalid hint");
    }
}

/////////////////////////////////////////////////////////////////////////////
// Message wrappers

void ilG_context_free(ilG_context *self)
{
    assert(!self->running);
    self->complete = 0;

    il_value nil = il_value_nil();
    ilE_handler_fire(self->destroy, &nil);
    il_value_free(nil);

    for (unsigned i = 0; i < self->manager.renderers.length; i++) {
        ilG_renderer *r = &self->manager.renderers.data[i];
        r->free(r->obj);
    }
    for (unsigned i = 0; i < self->manager.multirenderers.length; i++) {
        ilG_multirenderer *r = &self->manager.multirenderers.data[i];
        r->first.free(r->first.obj);
        free(r->second.types);
    }
    for (unsigned i = 0; i < self->manager.names.length; i++) {
        free(self->manager.names.data[i]);
    }
    for (unsigned i = 0; i < self->manager.coordsystems.length; i++) {
        ilG_coordsys *c = &self->manager.coordsystems.data[i];
        c->free(c->obj);
    }
    IL_FREE(self->manager.renderers);
    IL_FREE(self->manager.multirenderers);
    IL_FREE(self->manager.sinks);
    IL_FREE(self->manager.storages);
    IL_FREE(self->manager.namelinks);
    IL_FREE(self->manager.names);
    IL_FREE(self->manager.coordsystems);

    free(self->texunits);
    ilE_unregister(self->tick, self->tick_id);
    ilE_handler_destroy(self->tick);
    ilE_handler_destroy(self->resize);
    ilE_handler_destroy(self->close);
    queue_free(self->queue);

    SDL_GL_DeleteContext(self->context);
    SDL_DestroyWindow(self->window);
    free(self);
}

bool ilG_context_upload(ilG_context *self, void (*fn)(void*), void* ptr)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_UPLOAD;
    msg->value.upload.cb = fn;
    msg->value.upload.ptr = ptr;
    produce(self->queue, msg);
    return true;
}

bool ilG_context_resize(ilG_context *self, int w, int h)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_RESIZE;
    msg->value.resize[0] = w;
    msg->value.resize[1] = h;
    produce(self->queue, msg);
    return true;
}

char *strdup(const char*);
bool ilG_context_rename(ilG_context *self, const char *title)
{
    SDL_SetWindowTitle(self->window, title);
    return true;
}

void ilG_handle_addRenderer(ilG_handle self, ilG_handle node)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_ADD_RENDERER;
    msg->value.renderer.parent = self.id;
    msg->value.renderer.child = node.id;
    produce(self.context->queue, msg);
}

void ilG_handle_delRenderer(ilG_handle self, ilG_handle node)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_DEL_RENDERER;
    msg->value.renderer.parent = self.id;
    msg->value.renderer.child = node.id;
    produce(self.context->queue, msg);
}

void ilG_handle_addLight(ilG_handle self, struct ilG_light light)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_ADD_LIGHT;
    msg->value.light.parent = self.id;
    msg->value.light.child = light;
    produce(self.context->queue, msg);
}

void ilG_handle_delLight(ilG_handle self, struct ilG_light light)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_DEL_LIGHT;
    msg->value.light.parent = self.id;
    msg->value.light.child = light;
    produce(self.context->queue, msg);
}

void ilG_handle_message(ilG_handle self, int type, il_value data)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_MESSAGE;
    msg->value.message.node = self.id;
    msg->value.message.type = type;
    msg->value.message.data = data;
    produce(self.context->queue, msg);
}

void ilG_context_bindFB(ilG_context *self)
{
    if (self->use_default_fb) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        static const GLenum drawbufs[] = {
            GL_COLOR_ATTACHMENT0,   // accumulation
            GL_COLOR_ATTACHMENT1,   // normal
            GL_COLOR_ATTACHMENT2,   // diffuse
            GL_COLOR_ATTACHMENT3    // specular
        };
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, self->framebuffer);
        glDrawBuffers(4, &drawbufs[0]);
    }
}

void ilG_context_bind_for_outpass(ilG_context *self)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, self->framebuffer);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
}

/////////////////////////////////////////////////////////////////////////////
// Message handlers

static int context_upload(ilG_context *self, void (*fn)(void*), void* ptr)
{
    (void)self;
    fn(ptr);
    return 1;
}

static int context_resize(ilG_context *self, int w, int h)
{
    self->width = w;
    self->height = h;
    if (self->use_default_fb) {
        il_value val = il_value_vectorl(2, il_value_int(self->width), il_value_int(self->height));
        ilE_handler_fire(self->resize, &val);
        il_value_free(val);
        self->valid = 1;
        return 1;
    }

    // GL setup
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    IL_GRAPHICS_TESTERROR("Error setting up screen");

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

static void context_message(ilG_context *self, ilG_rendid id, int type, il_value data)
{
    ilG_msgsink *s = ilG_context_findSink(self, id);
    ilG_renderer *r = ilG_context_findRenderer(self, id);
    assert(s && r);
    s->fn(r->obj, type, &data);
    il_value_free(data);
}

ilG_renderer *ilG_context_findRenderer(ilG_context *self, ilG_rendid id)
{
    ilG_rendid key;
    unsigned idx;
    IL_FIND(self->manager.rendids, key, key == id, idx);
    if (idx < self->manager.rendids.length) {
        return &self->manager.renderers.data[idx];
    }
    return NULL;
}

ilG_msgsink *ilG_context_findSink(ilG_context *self, ilG_rendid id)
{
    ilG_msgsink r;
    unsigned idx;
    IL_FIND(self->manager.sinks, r, r.id == id, idx);
    if (idx < self->manager.sinks.length) {
        return &self->manager.sinks.data[idx];
    }
    return NULL;
}

il_table *ilG_context_findStorage(ilG_context *self, ilG_rendid id)
{
    ilG_rendstorage r;
    unsigned idx;
    IL_FIND(self->manager.storages, r, r.first == id, idx);
    if (idx < self->manager.storages.length) {
        return &self->manager.storages.data[idx].second;
    }
    return NULL;
}

const char *ilG_context_findName(ilG_context *self, ilG_rendid id)
{
    ilG_rendname r;
    unsigned idx;
    IL_FIND(self->manager.namelinks, r, r.first == id, idx);
    if (idx < self->manager.namelinks.length) {
        return self->manager.names.data[r.second];
    }
    return NULL;
}

unsigned ilG_context_addRenderer(ilG_context *self, ilG_rendid id, ilG_builder builder)
{
    ilG_buildresult b;
    bool res = builder.build(builder.obj, id, self, &b);
    if (res) {
        ilG_renderer r = (ilG_renderer) {
            .free = b.free,
            .update = b.update,
            .rchildren = {0,0,0},
            .mchildren = {0,0,0},
            .lights = {0,0,0},
            .obj = b.obj
        };
        if (b.draw) {
            ilG_multirenderer m = (ilG_multirenderer) {
                .first = r,
                .second = {
                    .draw = b.draw,
                    .coordsys = 0, // TODO: Select coord system
                    .types = b.types,
                    .num_types = b.num_types
                }
            };
            IL_APPEND(self->manager.multirenderers, m);
            IL_APPEND(self->manager.multirendids, id);
        } else {
            IL_APPEND(self->manager.renderers, r);
            IL_APPEND(self->manager.rendids, id);
        }
        return self->manager.renderers.length - 1;
    }
    return UINT_MAX;
}

unsigned ilG_context_addSink(ilG_context *self, ilG_rendid id, ilG_message_fn sink)
{
    ilG_msgsink s = {
        .fn = sink,
        .id = id
    };
    IL_APPEND(self->manager.sinks, s);
    return self->manager.sinks.length - 1;
}

unsigned ilG_context_addChild(ilG_context *self, ilG_rendid parent, ilG_rendid child)
{
    ilG_renderer *r = ilG_context_findRenderer(self, parent);
    unsigned idx;
    ilG_rendid id;
    IL_FIND(self->manager.rendids, id, id == child, idx);
    if (idx < self->manager.rendids.length) {
        IL_APPEND(r->rchildren, idx);
        return 1;
    }
    IL_FIND(self->manager.multirendids, id, id == child, idx);
    if (idx < self->manager.multirendids.length) {
        IL_APPEND(r->mchildren, idx);
        return 1;
    }
    il_error("No such renderer %u", child);
    return 0;
}

unsigned ilG_context_addLight(ilG_context *self, ilG_rendid id, ilG_light light)
{
    ilG_renderer *r = ilG_context_findRenderer(self, id);
    IL_APPEND(r->lights, light);
    return r->lights.length-1;
}

unsigned ilG_context_addStorage(ilG_context *self, ilG_rendid id)
{
    ilG_rendstorage r = {
        .first = id,
        .second = il_table_new()
    };
    IL_APPEND(self->manager.storages, r);
    return self->manager.storages.length - 1;
}

unsigned ilG_context_addName(ilG_context *self, ilG_rendid id, const char *name)
{
    unsigned idx;
    IL_FIND(self->manager.names, const char *s, strcmp(s, name) == 0, idx);
    if (idx == self->manager.names.length) {
        char *s = strdup(name);
        IL_APPEND(self->manager.names, s);
    }
    ilG_rendname r = {
        .first = id,
        .second = idx
    };
    IL_APPEND(self->manager.namelinks, r);
    return self->manager.namelinks.length - 1;
}

bool ilG_context_delRenderer(ilG_context *self, ilG_rendid id)
{
    unsigned idx;
    ilG_rendid key;
    IL_FIND(self->manager.rendids, key, key == id, idx);
    if (idx < self->manager.rendids.length) {
        IL_FASTREMOVE(self->manager.renderers, idx);
        IL_FASTREMOVE(self->manager.rendids, idx);
        return true;
    }
    IL_FIND(self->manager.multirendids, key, key == id, idx);
    if (idx < self->manager.multirendids.length) {
        IL_FASTREMOVE(self->manager.multirenderers, idx);
        IL_FASTREMOVE(self->manager.multirendids, idx);
        return true;
    }
    return false;
}

bool ilG_context_delSink(ilG_context *self, ilG_rendid id)
{
    unsigned idx;
    ilG_msgsink r;
    IL_FIND(self->manager.sinks, r, r.id == id, idx);
    if (idx < self->manager.sinks.length) {
        IL_FASTREMOVE(self->manager.sinks, idx);
        return true;
    }
    return false;
}

bool ilG_context_delChild(ilG_context *self, ilG_rendid parent, ilG_rendid child)
{
    unsigned idx;
    ilG_rendid id;
    ilG_renderer *par = ilG_context_findRenderer(self, parent);
    IL_FIND(self->manager.rendids, id, id == child, idx);
    if (idx < self->manager.rendids.length) {
        unsigned idx2;
        IL_FIND(par->rchildren, id, id == idx, idx2);
        if (idx2 < par->rchildren.length) {
            IL_FASTREMOVE(par->rchildren, idx2);
            return true;
        } else {
            return false;
        }
    }
    IL_FIND(self->manager.multirendids, id, id == child, idx);
    if (idx < self->manager.multirendids.length) {
        unsigned idx2;
        IL_FIND(par->mchildren, id, id == idx, idx2);
        if (idx2 < par->mchildren.length) {
            IL_FASTREMOVE(par->mchildren, idx2);
            return true;
        } else {
            return false;
        }
    }
    return false;
}

bool ilG_context_delStorage(ilG_context *self, ilG_rendid id)
{
    unsigned idx;
    ilG_rendstorage r;
    IL_FIND(self->manager.storages, r, r.first == id, idx);
    if (idx < self->manager.storages.length) {
        IL_FASTREMOVE(self->manager.storages, idx);
        return true;
    }
    return false;
}

bool ilG_context_delName(ilG_context *self, ilG_rendid id)
{
    unsigned idx;
    ilG_rendname r;
    IL_FIND(self->manager.namelinks, r, r.first == id, idx);
    if (idx < self->manager.namelinks.length) {
        IL_REMOVE(self->manager.namelinks, idx);
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////
// Rendering logic and context setup

static void render_renderer(ilG_context *context, ilG_renderer *par)
{
    ilG_rendermanager *rm = &context->manager;
    ilG_renderer *r;
    ilG_multirenderer *m;
    for (unsigned i = 0, len = par->rchildren.length; i < len; i++) {
        r = &rm->renderers.data[par->rchildren.data[i]];
        ilG_rendid id = rm->rendids.data[par->rchildren.data[i]];
        r->update(r->obj, id);
        ilG_testError("In %s draw", ilG_context_findName(context, id));
        render_renderer(context, r);
    }
    for (unsigned i = 0, len = par->mchildren.length; i < len; i++) {
        m = &rm->multirenderers.data[par->mchildren.data[i]];
        ilG_rendid id = rm->multirendids.data[par->mchildren.data[i]];
        m->first.update(m->first.obj, id);
        ilG_testError("In %s draw", ilG_context_findName(context, id));
        //m->second.draw(m->first.obj, m->first.id);
        ilG_testError("In %s multidraw", ilG_context_findName(context, id));
        render_renderer(context, &m->first);
    }
}

static void render_frame(ilG_context *context)
{
    glViewport(0, 0, context->width, context->height);
    context->material       = NULL;
    context->materialb      = NULL;
    context->drawable       = NULL;
    context->drawableb      = NULL;
    context->texture        = NULL;
    context->textureb       = NULL;
    context->positionable   = NULL;

    il_debug("Begin render");
    ilG_context_bindFB(context);
    if (context->debug_render) {
        glClearColor(0.39, 0.58, 0.93, 1.0); // cornflower blue
    } else {
        glClearColor(0, 0, 0, 1.0);
    }
    ilG_testError("glClearColor");
    glClearDepth(1.0);
    ilG_testError("glClearDepth");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // asserts that first renderer is the context itself
    render_renderer(context, 0);
}

static void measure_frametime(ilG_context *context)
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
static /*APIENTRY*/ GLvoid error_cb(GLenum source, GLenum type, GLuint id, GLenum severity,
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

static void setup_context(ilG_context *self) // main thread
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
    //ilG_registerInputBackend(self);
    SDL_SetWindowData(self->window, "context", self);
}

static void setup_glew(ilG_context *self)
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
    if (!self->use_default_fb) {
        glGenFramebuffers(1, &self->framebuffer);
        glGenTextures(5, &self->fbtextures[0]);
        ilG_testError("Unable to generate framebuffer");
    }
    self->complete = 1;
}

static void *render_thread(void *ptr)
{
    ilG_context *self = ptr;

    self->context = SDL_GL_CreateContext(self->window);

    SDL_GL_SetSwapInterval(self->vsync);
    setup_glew(self);
    context_resize(self, self->startWidth, self->startHeight);

    while (self->valid) {
        // Run per-frame stuff
        il_value nil = il_value_nil();
        ilE_handler_fire(self->tick, &nil);
        il_value_free(nil);
        // Process messages
        struct ilG_context_msg *msg;
        while ((msg = consume(self->queue))) {
            switch (msg->type) {
            case ILG_UPLOAD: context_upload(self, msg->value.upload.cb, msg->value.upload.ptr); break;
            case ILG_RESIZE: context_resize(self, msg->value.resize[0], msg->value.resize[1]); break;
            case ILG_STOP: goto stop;
            case ILG_MESSAGE: context_message(self, msg->value.message.node, msg->value.message.type, msg->value.message.data); break;
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
            done_consume(self->queue);
        }
        int width, height;
        SDL_GetWindowSize(self->window, &width, &height);
        if (width != self->width || height != self->height) {
            context_resize(self, width, height);
        }
        // Render
        render_frame(self);
        if (self->use_default_fb) {
            glViewport(0,0, self->width, self->height);
            SDL_GL_SwapWindow(self->window);
        }
        // Perform fps measuring calculations
        measure_frametime(self);
    }

    if (!self->valid || !self->complete) {
        il_error("Tried to render invalid context");
    }

stop:
    done_consume(self->queue);
    glDeleteFramebuffers(1, &self->framebuffer);
    glDeleteTextures(5, &self->fbtextures[0]);
    return NULL;
}

bool ilG_context_start(ilG_context *self)
{
    setup_context(self);

    self->running = true;
    // Start thread
    int res = pthread_create(&self->thread, NULL, render_thread, self);
    if (res) {
        il_error("pthread_create: %s", strerror(errno));
    }
    return res == 0;
}

void ilG_context_stop(ilG_context *self)
{
    struct ilG_context_msg *msg = calloc(1, sizeof(struct ilG_context_msg));
    msg->type = ILG_STOP;
    produce(self->queue, msg);
    pthread_join(self->thread, NULL);
    self->running = false;
}

static void tabulate(unsigned n)
{
    for (unsigned i = 0; i < 2*n; i++) {
        fputc(' ', stderr);
    }
}

/*void ilG_context_recursivePrint(ilG_context *self, ilG_rendid id, unsigned depth)
{
    tabulate(depth); fprintf(stderr, "%u:\n", id);
    tabulate(depth+1); fprintf(stderr, "sink: %s\n", ilG_context_findSink(self, id)? "true" : "false");
    ilG_renderer *r;
    unsigned iter;
    while ((r = ilG_context_iterChildren(self, id, &iter))) {
        ilG_context_recursivePrint(self, r->id, depth+1);
    }
}*/

/*void ilG_context_printScenegraph(ilG_context *self)
{
    fprintf(stderr, "storage:\n");
    il_table_show(&self->storage);
    fprintf(stderr, "renderers:\n");
    ilG_rendermanager *rm = &self->manager;
    for (unsigned i = 0; i < rm->names.length; i++) {
        fprintf(stderr, "  %s: ", rm->names.data[i]);
        for (unsigned j = 0; j < rm->namelinks.length; j++) {
            if (rm->namelinks.data[j].second == i) {
                fprintf(stderr, "%u, ", rm->namelinks.data[j].first);
            }
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "  No name: ");
    for (unsigned i = 0; i < rm->renderers.length; i++) {
        bool has_name = false;
        for (unsigned j = 0; j < rm->namelinks.length; j++) {
            if (rm->namelinks.data[j].first == rm->renderers.data[i].id) {
                has_name = true;
            }
        }
        if (!has_name) {
            fprintf(stderr, "%u, ", rm->renderers.data[i].id);
        }
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "hierarchy:\n");
    //ilG_context_recursivePrint(self, 0, 1);
    }*/
