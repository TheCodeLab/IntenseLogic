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

static IL_ARRAY(ilE_handler*,) persistent_handlers;

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
    event_active(e, 0, 0);
}

static void append_before_afters(ilE_handler *self, struct callback cb)
{
    ssize_t i;
    for (i = self->callbacks.length - 1; i > 0; i--) {
        if (self->callbacks.data[i].behaviour != ILE_AFTER) {
            break;
        }
    }
    IL_INSERT(self->callbacks, (size_t)i, cb);
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

    if (self->callbacks.length < 1) {
        IL_SET(self->callbacks, 0, cb); // first element, no need to sort list
    } else if (behaviour == ILE_BEFORE) {
        // For this case, we want to move an element that doesn't care where it is out of the way, to make room for one that wants to run at the start.
        // B = ILE_BEFORE
        // A = ILE_AFTER
        // D = ILE_DONTCARE
        // M = Where we want to be
        // N = New spot for element we had to shift out of the way to get where we want to be
        // BBBB M DDDDDDDDDDDDDD N AAAA
        size_t i;
        // find M
        for (i = 0; i < self->callbacks.length; i++) {
            if (self->callbacks.data[i].behaviour != ILE_BEFORE) {
                break;
            }
        }
        if (i < self->callbacks.length // M exists
            && self->callbacks.data[i].behaviour == ILE_DONTCARE) // and M can be swapped to another spot
        {
            append_before_afters(self, self->callbacks.data[i]);
            IL_SET(self->callbacks, i, cb);
        } else if (i < self->callbacks.length) { // M exists, but can't be swapped (there are no DONTCAREs)
            IL_INSERT(self->callbacks, i, cb);
        }
    } else if (behaviour == ILE_DONTCARE) {
        append_before_afters(self, cb);
    } else if (behaviour == ILE_AFTER) {
        IL_APPEND(self->callbacks, cb);
    } else {
        il_error("Unknown enum value %u", behaviour);
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

static void showRemaining(const ilE_handler *hnd, size_t size, const void *data, void *ctx)
{
    (void)hnd, (void)size, (void)data, (void)ctx;
    ilE_dumpAll();
}

static int shutdownCallbacks_id, shutdownHandlers_id, showRemaining_id;

void ilE_init()
{
    ilE_base = event_base_new();
    ilE_shutdown = ilE_handler_new_with_name("il.event.shutdown");
    shutdownCallbacks_id = ilE_register(ilE_shutdown, ILE_AFTER, ILE_ANY, shutdownCallbacks, NULL);
    ilE_shutdownCallbacks = ilE_handler_new_with_name("il.event.shutdownCallbacks");
    shutdownCallbacks_id = ilE_register(ilE_shutdownCallbacks, ILE_AFTER, ILE_ANY, shutdownHandlers, NULL);
    ilE_shutdownHandlers = ilE_handler_new_with_name("il.event.shutdownHandlers");
    showRemaining_id = ilE_register(ilE_shutdownHandlers, ILE_AFTER, ILE_ANY, showRemaining, NULL);
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
    fprintf(stderr, "Registry dump of handler %s <%p>:\n", self->name, self);
    for (i = 0; i < self->callbacks.length; i++) {
        struct callback *cb = &self->callbacks.data[i];
        char behaviour_str[64], threading_str[64];
#define e2s(p, e, s) case e: strcpy(p, s); break
#define b2s(n) e2s(behaviour_str, ILE_##n, #n)
        switch (cb->behaviour) {
            b2s(DONTCARE);
            b2s(BEFORE);
            b2s(AFTER);
            b2s(OVERRIDE);
            default:
            strcpy(behaviour_str, "???");
        }
#undef b2s
#define t2s(n) e2s(threading_str, ILE_##n, #n)
        switch (cb->threading) {
            t2s(ANY);
            t2s(MAIN);
            t2s(TLS);
            default:
            strcpy(behaviour_str, "???");
        }
#undef t2s
#undef e2s
        fprintf(stderr, "\t%s <%p> behaviour:%s threading:%s ctx<%p>\n", cb->name, cb->callback, behaviour_str, threading_str, cb->ctx);
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

