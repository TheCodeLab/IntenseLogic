#include "event.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#include "util/log.h"
#include "util/array.h"
#include "util/uthash.h"
#include "util/ilassert.h"

char *strdup(const char*);

struct ilE_handler_callback {
    ilE_callback callback;
    int priority;
    enum ilE_threading threading;
    char name[128];
    int handle;
    il_value ctx;
};

ilE_handler* ilE_handler_new()
{
    ilE_handler* h = calloc(1, sizeof(ilE_handler));
    ilE_handler_init(h);
    return h;
}

void ilE_handler_init(ilE_handler *self)
{
    memset(self, 0, sizeof(ilE_handler));
    sprintf(self->name, "Handler %p", (void*)self);
}

ilE_handler *ilE_handler_new_with_name(const char *name)
{
    ilE_handler *h = ilE_handler_new();
    ilE_handler_name(h, name);
    return h;
}

void ilE_handler_init_with_name(ilE_handler *self, const char *name)
{
    ilE_handler_init(self);
    ilE_handler_name(self, name);
}

void ilE_handler_destroy(ilE_handler *self)
{
    IL_FREE(self->callbacks);
}

const char *ilE_handler_getName(const ilE_handler *self)
{
    return self->name;
}

void ilE_handler_name(ilE_handler *self, const char *name)
{
    strcpy(self->name, name);
}

void ilE_handler_fire(ilE_handler *self, const il_value *data)
{
    unsigned i;
    il_debug("Dispatch: %s <%p> (data: %p)", self->name, (void*)self, (void*)data); // TODO: Print contents of data
    for (i = 0; i < self->callbacks.length; i++) {
        if (self->callbacks.data[i].callback) {
            self->callbacks.data[i].callback(data, &self->callbacks.data[i].ctx);
        }
    }
}

void ilE_handler_fire_once(ilE_handler *self, const il_value *data)
{
    ilE_handler_fire(self, data);
    ilE_handler_destroy(self);
}

int ilE_register_real(ilE_handler* self, const char *name, int priority, enum ilE_threading threads, ilE_callback callback, il_value ctx)
{
    struct ilE_handler_callback cb;
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
    il_error("No callback %i in handler %s <%p>", handle, self->name, (void*)self);
}

void ilE_dump(ilE_handler *self)
{
    size_t i;
    fprintf(stderr, "Registry dump of handler %s <%p>:\n", self->name, (void*)self);
    for (i = 0; i < self->callbacks.length; i++) {
        struct ilE_handler_callback *cb = &self->callbacks.data[i];
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
#ifndef WIN32
        fprintf(stderr, "\t%s <%p> priority:%i threading:%s ctx<%p>\n", cb->name, cb->callback, cb->priority, threading_str, (void*)&cb->ctx);// TODO: Print contents of ctx
#endif
    }
}
