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
#include "util/assert.h"

char *strdup(const char*);

static void ilE_handler_fire_forced(ilE_handler *self, size_t size, const void *data);

struct event_base *ilE_base;

struct ilE_event {
    ilE_handler *handler;
    uint8_t size;
    char data[];
};

struct callback {
    ilE_callback callback;
    enum ilE_behaviour behaviour;
    enum ilE_threading threading;
    char name[128];
    int handle;
    void * ctx;
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
    ilE_handler_fire_forced(ev->handler, ev->size, ev->data);
    free(ev);
}

static void timer_dispatch(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd, (void)events;
    ilE_handler *hnd = (ilE_handler*)ctx;
    ilE_handler_fire_forced(hnd, 0, NULL);
}

static void file_dispatch(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd, (void)events;
    ilE_handler *hnd = (ilE_handler*)ctx;
    int socktype;
    socklen_t len;
    void *data;
    size_t size;

    getsockopt(fd, SOL_SOCKET, SO_TYPE, &socktype, &len);
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
    ilE_handler_fire_forced(hnd, size, data);
    free(data);
}

ilE_handler* ilE_handler_new()
{
    ilE_handler* h = calloc(1, sizeof(ilE_handler));
    h->type = NORMAL_HND;
    sprintf(h->name, "Normal Handler <%p>", h);
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
    sprintf(h->name, "Timer Handler <%p>: %fs interval", h, tv->tv_sec + tv->tv_usec/1000000.0);
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
    h->ev = event_new(ilE_base, fd, events, &file_dispatch, h);
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
    sprintf(h->name, "File Watcher <%p>: Watching %s on %i", h, what_str, fd);
    return h;
}

void ilE_handler_destroy(ilE_handler *self)
{
    if (self->callbacks.length > 0) {
        il_warning("%zu callbacks still registered for %s <%p>", self->callbacks.length, self->name, self);
    }
    IL_FREE(self->callbacks);
    event_free(self->ev);
}

const char *ilE_handler_getName(const ilE_handler *self)
{
    return self->name;
}

void ilE_handler_name(ilE_handler *self, const char *name)
{
    strcpy(self->name, name);
}

static void ilE_handler_fire_forced(ilE_handler *self, size_t size, const void *data)
{
    unsigned i;
    il_debug("Dispatch: %s <%p> (data: %p, size: %zu)", self->name, self, data, size);
    for (i = 0; i < self->callbacks.length; i++) {
        if (self->callbacks.data[i].callback) {
            self->callbacks.data[i].callback(self, size, data, self->callbacks.data[i].ctx);
        }
    }
}

void ilE_handler_fire(ilE_handler *self, size_t size, const void *data)
{
    il_return_on_fail(self->type == NORMAL_HND);
    ilE_handler_fire_forced(self, size, data);
}

void ilE_handler_fireasync(ilE_handler *self, size_t size, const void *data)
{
    il_return_on_fail(self->type == NORMAL_HND);
    struct ilE_event *ev = calloc(1, sizeof(struct ilE_event) + size);
    ev->handler = self;
    ev->size = size;
    memcpy(ev->data, data, size);
    struct event *e = event_new(ilE_base, -1, 0, normal_dispatch, ev);
    event_add(e, NULL);
}

int ilE_register_real(ilE_handler* self, const char *name, enum ilE_behaviour behaviour, enum ilE_threading threads, ilE_callback callback, void * ctx)
{
    struct callback cb; 
    cb.callback = callback; 
    cb.behaviour = behaviour; 
    cb.threading = threads;
    cb.handle = self->cur_id++;
    cb.ctx = ctx;
    strcpy(cb.name, name);

    if (behaviour == ILE_OVERRIDE) {
        // TODO: Fix memory leak here
        self->callbacks.length = 0;
    }

    if (behaviour == ILE_BEFORE) {
        if (self->callbacks.length < 1) {
            IL_SET(self->callbacks, 0, cb); // first
        } else {
            if (self->callbacks.data[0].behaviour == ILE_DONTCARE) {
                // move the first element to the last spot so we can get at it
                IL_APPEND(self->callbacks, self->callbacks.data[0]);
            } else {
                // we shift the entire list to the right
                struct callback * unused;
                IL_INDEXORZERO(self->callbacks, self->callbacks.length, unused);
                memmove(&self->callbacks.data[1], &self->callbacks.data[0], sizeof(struct callback) * (self->callbacks.length-1));
                IL_SET(self->callbacks, 0, cb);
            }
        }
    } else {
        // can be appended
        IL_APPEND(self->callbacks, cb);
    }

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

static void shutdownCallbacks(const ilE_handler *hnd, size_t size, const void *data, void *ctx)
{
    (void)hnd, (void)size, (void)data, (void)ctx;
    ilE_handler_fire(ilE_shutdownCallbacks, 0, NULL);
}

static void shutdownHandlers(const ilE_handler *hnd, size_t size, const void *data, void *ctx)
{
    (void)hnd, (void)size, (void)data, (void)ctx;
    ilE_handler_fire(ilE_shutdownHandlers, 0, NULL);
}

void ilE_init()
{
    ilE_base = event_base_new();
    ilE_shutdown = ilE_handler_new();
    ilE_register(ilE_shutdown, ILE_AFTER, ILE_ANY, shutdownCallbacks, NULL);
    ilE_shutdownCallbacks = ilE_handler_new();
    ilE_register(ilE_shutdownCallbacks, ILE_AFTER, ILE_ANY, shutdownHandlers, NULL);
    ilE_shutdownHandlers = ilE_handler_new();
}

void ilE_quit()
{
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
    fprintf(stderr, "Registry dump of handler %s <%p>:\n", self->name, self);
    for (i = 0; i < self->callbacks.length; i++) {
        struct callback *cb = &self->callbacks.data[i];
        fprintf(stderr, "\t%s <%p> behaviour:%u threading:%u ctx<%p>\n", cb->name, cb->callback, cb->behaviour, cb->threading, cb->ctx);
    }
    fprintf(stderr, "End of registry dump.\n");
}

ilE_handler *ilE_shutdown;
ilE_handler *ilE_shutdownCallbacks;
ilE_handler *ilE_shutdownHandlers;

