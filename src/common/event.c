#include "event.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#include "common/base.h"
#include "util/log.h"
#include "util/array.h"
#include "util/uthash.h"
#include "util/ilassert.h"

char *strdup(const char*);

static void ilE_handler_fire_forced(ilE_handler *self, const il_value *data);

struct ilE_event {
    ilE_handler *handler;
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

struct ilE_handler {
    IL_ARRAY(struct callback, callback_array) callbacks;
    struct event *ev;
    char name[128];
    int cur_id;
};

ilE_handler* ilE_handler_new()
{
    ilE_handler* h = calloc(1, sizeof(ilE_handler));
    sprintf(h->name, "Normal Handler");
    return h;
}

ilE_handler *ilE_handler_new_with_name(const char *name)
{
    ilE_handler *h = ilE_handler_new();
    ilE_handler_name(h, name);
    return h;
}

void ilE_handler_destroy(ilE_handler *self)
{
    if (self->callbacks.length > 0) {
        il_warning("%zu callbacks still registered for %s <%p>", self->callbacks.length, self->name, self);
    }
    IL_FREE(self->callbacks);
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
    ilE_handler_fire_forced(self, data);
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

static int shutdownCallbacks_id, shutdownHandlers_id;

void ilE_init()
{
    ilE_shutdown = ilE_handler_new_with_name("il.event.shutdown");
    shutdownCallbacks_id = ilE_register(ilE_shutdown, ILE_AFTER, ILE_ANY, shutdownCallbacks, il_value_nil());
    ilE_shutdownCallbacks = ilE_handler_new_with_name("il.event.shutdownCallbacks");
    shutdownCallbacks_id = ilE_register(ilE_shutdownCallbacks, ILE_AFTER, ILE_ANY, shutdownHandlers, il_value_nil());
    ilE_shutdownHandlers = ilE_handler_new_with_name("il.event.shutdownHandlers");
}

void ilE_quit()
{
    ilE_unregister(ilE_shutdown, shutdownCallbacks_id);
    ilE_unregister(ilE_shutdownCallbacks, shutdownHandlers_id);
    ilE_handler_destroy(ilE_shutdown);
    ilE_handler_destroy(ilE_shutdownCallbacks);
    ilE_handler_destroy(ilE_shutdownHandlers);
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

ilE_handler *ilE_shutdown;
ilE_handler *ilE_shutdownCallbacks;
ilE_handler *ilE_shutdownHandlers;
