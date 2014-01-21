#include "event.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <unistd.h>
#include <errno.h>

#include "common/base.h"
#include "util/log.h"
#include "util/array.h"
#include "util/uthash.h"
#include "util/ilassert.h"

#ifdef WIN32
#include <winsock2.h>
#endif

char *strdup(const char*);

static void ilE_handler_fire_forced(ilE_handler *self, const il_value *data);

struct event_base *ilE_base;

static IL_ARRAY(ilE_handler*,) persistent_handlers;

struct ilE_event {
    ilE_handler *handler;
    uint8_t size;
    il_value data;
};

struct callback {
    ilE_callback callback;
    int priority;
    enum ilE_threading threading;
    char name[128];
    int handle;
    il_value ctx;
};

enum handler_type {
    NORMAL_HND,
    FILE_HND,
    TIMER_HND
};

struct ilE_handler {
    IL_ARRAY(struct callback, callback_array) callbacks;
    enum handler_type type;
    struct event *ev;
    char name[128];
    int cur_id;
    union {
        struct {
            int fd;
            enum ilE_fdevent fdevent;
        } file;
        struct timeval tv;
    } data;
};

static void normal_dispatch(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd, (void)events;
    struct ilE_event *ev = (struct ilE_event*)ctx;
    ilE_handler_fire_forced(ev->handler, &ev->data);
    il_value_free(ev->data);
    free(ev);
}

static void timer_dispatch(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd, (void)events;
    ilE_handler *hnd = (ilE_handler*)ctx;
    il_value nil = il_value_nil();
    ilE_handler_fire_forced(hnd, &nil);
}

static void file_dispatch(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd, (void)events;
    ilE_handler *hnd = (ilE_handler*)ctx;
    int socktype;
#ifdef WIN32
# define cast (char*)
    int len;
#else
# define cast
    socklen_t len;
#endif
    void *data;
    size_t size;

    getsockopt(fd, SOL_SOCKET, SO_TYPE, cast &socktype, &len);
#undef cast
    if (hnd->data.file.fdevent == ILE_READASARG && socktype == SOCK_STREAM) {
        struct evbuffer *evbuf = evbuffer_new();
        int res;
        do {
            char buf[4096];
            res = read(fd, buf, 4096);
            evbuffer_add(evbuf, buf, res);
        } while(res > 0);
        data = malloc(size = evbuffer_get_length(evbuf));
        evbuffer_remove(evbuf, data, size);
        evbuffer_free(evbuf);
    } else if (hnd->data.file.fdevent == ILE_READASARG && socktype == SOCK_DGRAM) {
        char *buf = malloc(64*1024-8);
        ssize_t res = recv(fd, buf, 64*1024-8, 0);
        if (res <= 0) {
            il_error("recv: %s", strerror(errno));
            return;
        }
        data = buf;
        size = (size_t)res;
    } else {
        il_error("Socket is not SOCK_DGRAM or SOCK_STREAM");
        return;
    }
    il_value val = il_value_opaque(il_opaque(data,free));
    ilE_handler_fire_forced(hnd, &val);
}

ilE_handler* ilE_handler_new()
{
    ilE_handler* h = calloc(1, sizeof(ilE_handler));
    h->type = NORMAL_HND;
    sprintf(h->name, "Normal Handler");
    return h;
}

ilE_handler *ilE_handler_new_with_name(const char *name)
{
    ilE_handler *h = ilE_handler_new();
    ilE_handler_name(h, name);
    return h;
}

ilE_handler *ilE_handler_timer(const struct timeval *tv)
{
    ilE_handler *h = calloc(1, sizeof(ilE_handler));
    h->type = TIMER_HND;
    memcpy(&h->data.tv, tv, sizeof(struct timeval));
    h->ev = event_new(ilE_base, -1, EV_TIMEOUT|EV_PERSIST, &timer_dispatch, h);
    event_add(h->ev, tv);
    sprintf(h->name, "Timer Handler: %fs interval", tv->tv_sec + tv->tv_usec/1000000.0);
    IL_APPEND(persistent_handlers, h);
    return h;
}

ilE_handler *ilE_handler_watch(int fd, enum ilE_fdevent what)
{
    short events = 
        ((what & ILE_READ || what & ILE_READASARG)? EV_READ : 0) |
        ((what & ILE_WRITE)? EV_WRITE : 0);
    ilE_handler *h = calloc(1, sizeof(ilE_handler));
    h->type = FILE_HND;
    h->data.file.fd = fd;
    h->data.file.fdevent = what;
    h->ev = event_new(ilE_base, fd, events|EV_PERSIST, &file_dispatch, h);
    event_add(h->ev, NULL);
    char what_str[128] = {0};
    if (what & ILE_READ) {
        strcat(what_str, "read");
    }
    if (what & ILE_READASARG) {
        if (!(what & ILE_READ)) {
            strcat(what_str, "read");
        }
        strcat(what_str, " from buffer");
    }
    if (what & ILE_WRITE) {
        if ((what & ILE_READ) || (what & ILE_READASARG)) {
            strcat(what_str, " + ");
        }
        strcat(what_str, "write");
    }
    sprintf(h->name, "File Watcher: Watching %s on %i", what_str, fd);
    IL_APPEND(persistent_handlers, h);
    return h;
}

void ilE_handler_destroy(ilE_handler *self)
{
    unsigned i;
    for (i = 0; i < persistent_handlers.length; i++) {
        if (self == persistent_handlers.data[i]) {
            persistent_handlers.data[i] = persistent_handlers.data[--persistent_handlers.length];
        }
    }
    if (self->callbacks.length > 0) {
        il_warning("%zu callbacks still registered for %s <%p>", self->callbacks.length, self->name, self);
    }
    IL_FREE(self->callbacks);
    if (self->ev) {
        event_free(self->ev);
    }
    free(self);
}

const char *ilE_handler_getName(const ilE_handler *self)
{
    return self->name;
}

void ilE_handler_name(ilE_handler *self, const char *name)
{
    strcpy(self->name, name);
}

static void ilE_handler_fire_forced(ilE_handler *self, const il_value *data)
{
    unsigned i;
    il_debug("Dispatch: %s <%p> (data: %p)", self->name, self, data); // TODO: Print contents of data
    for (i = 0; i < self->callbacks.length; i++) {
        if (self->callbacks.data[i].callback) {
            self->callbacks.data[i].callback(data, &self->callbacks.data[i].ctx);
        }
    }
}

void ilE_handler_fire(ilE_handler *self, const il_value *data)
{
    il_return_on_fail(self->type == NORMAL_HND);
    ilE_handler_fire_forced(self, data);
}

void ilE_handler_fireasync(ilE_handler *self, il_value data)
{
    il_return_on_fail(self->type == NORMAL_HND);
    struct ilE_event *ev = calloc(1, sizeof(struct ilE_event));
    ev->handler = self;
    ev->data = data;
    struct event *e = event_new(ilE_base, -1, 0, normal_dispatch, ev);
    event_add(e, NULL);
    event_active(e, 0, 0);
}

int ilE_register_real(ilE_handler* self, const char *name, int priority, enum ilE_threading threads, ilE_callback callback, il_value ctx)
{
    struct callback cb; 
    unsigned i;

    cb.callback = callback;
    cb.priority = priority; 
    cb.threading = threads;
    cb.handle = self->cur_id++;
    cb.ctx = ctx;
    strcpy(cb.name, name);
    for (i = 0; i < self->callbacks.length; i++) {
        if (self->callbacks.data[i].priority > cb.priority) {
            IL_INSERT(self->callbacks, i, cb);
            return cb.handle;
        }
    }
    IL_APPEND(self->callbacks, cb);

    return cb.handle;
}

void ilE_unregister(ilE_handler *self, int handle)
{
    unsigned i;
    for (i = 0; i < self->callbacks.length; i++) {
        if (self->callbacks.data[i].handle == handle) {
            IL_REMOVE(self->callbacks, i);
            return;
        }
    }
    il_error("No callback %i in handler %s <%p>", handle, self->name, self);
}

static void shutdownCallbacks(const il_value *data, il_value *ctx)
{
    (void)data, (void)ctx;
    il_value nil = il_value_nil();
    ilE_handler_fire(ilE_shutdownCallbacks, &nil);
}

static void shutdownHandlers(const il_value *data, il_value *ctx)
{
    (void)data, (void)ctx;
    il_value nil = il_value_nil();
    ilE_handler_fire(ilE_shutdownHandlers, &nil);
}

static void showRemaining(const il_value *data, il_value *ctx)
{
    (void)data, (void)ctx;
    ilE_dumpAll();
}

static int shutdownCallbacks_id, shutdownHandlers_id, showRemaining_id;

void ilE_init()
{
    ilE_base = event_base_new();
    ilE_shutdown = ilE_handler_new_with_name("il.event.shutdown");
    shutdownCallbacks_id = ilE_register(ilE_shutdown, ILE_AFTER, ILE_ANY, shutdownCallbacks, il_value_nil());
    ilE_shutdownCallbacks = ilE_handler_new_with_name("il.event.shutdownCallbacks");
    shutdownCallbacks_id = ilE_register(ilE_shutdownCallbacks, ILE_AFTER, ILE_ANY, shutdownHandlers, il_value_nil());
    ilE_shutdownHandlers = ilE_handler_new_with_name("il.event.shutdownHandlers");
    showRemaining_id = ilE_register(ilE_shutdownHandlers, ILE_AFTER, ILE_ANY, showRemaining, il_value_nil());
}

void ilE_quit()
{
    ilE_unregister(ilE_shutdown, shutdownCallbacks_id);
    ilE_unregister(ilE_shutdownCallbacks, shutdownHandlers_id);
    ilE_unregister(ilE_shutdownHandlers, showRemaining_id);
    ilE_handler_destroy(ilE_shutdown);
    ilE_handler_destroy(ilE_shutdownCallbacks);
    ilE_handler_destroy(ilE_shutdownHandlers);
}

void ilE_loop()
{
    event_base_loop(ilE_base, 0);
}

void ilE_dump(ilE_handler *self)
{
    size_t i;
    fprintf(stderr, "Registry dump of handler %s <%p>:\n", self->name, (void*)self);
    for (i = 0; i < self->callbacks.length; i++) {
        struct callback *cb = &self->callbacks.data[i];
        char threading_str[64];
        switch (cb->threading) {
#define t2s(n) case ILE_##n: strcpy(threading_str, #n); break
            t2s(ANY);
            t2s(MAIN);
            t2s(TLS);
#undef t2s
            default:
            strcpy(threading_str, "???");
        }
        fprintf(stderr, "\t%s <%p> priority:%i threading:%s ctx<%p>\n", cb->name, (void*)cb->callback, cb->priority, threading_str, &cb->ctx);// TODO: Print contents of ctx
    }
}

void ilE_dumpAll()
{
    size_t i;
    if (persistent_handlers.length > 0) {
        fprintf(stderr, "=== Some persistent handlers have not been destroyed, preventing shutdown:\n");
    }
    for (i = 0; i < persistent_handlers.length; i++) {
        if (persistent_handlers.data[i]) {
            ilE_dump(persistent_handlers.data[i]);
        }
    }
}

ilE_handler *ilE_shutdown;
ilE_handler *ilE_shutdownCallbacks;
ilE_handler *ilE_shutdownHandlers;

