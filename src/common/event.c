#include "event.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <event2/event.h>

#include "common/base.h"
#include "util/log.h"
#include "common/array.h"
#include "util/uthash.h"

static struct event_base *ilE_base;
ilE_registry *il_registry;

enum flags {
    LIFETIME_INFINITE = 1
};

struct ilE_event {
    char *name;
    uint8_t size;
    uint8_t flags;
    char data[];
};

struct callback {
    ilE_callback callback;
    enum ilE_behaviour behaviour;
    enum ilE_threading threading;
    void * ctx;
};

struct callbacks {
    char *name;
    IL_ARRAY(struct callback, callback_array) arr;
    UT_hash_handle hh;
};

struct ilE_registry {
    struct callbacks *callbacks;
    ilE_registry *parent;
};

struct dispatch_ctx {
    struct ilE_event *ev;
    ilE_registry *registry;
};

static void dispatch(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd;
    (void)events;
    struct ilE_event* ev = ((struct dispatch_ctx*)ctx)->ev;
    ilE_registry* registry = ((struct dispatch_ctx*)ctx)->registry;
    unsigned i;
    // figure out which entry in our callbacks array array is for this event
    struct callbacks* callbacks = NULL;

    il_debug("Dispatch: %s", ev->name);
    while (registry) {
        //HASH_FIND_STR(registry->callbacks, ev->name, callbacks);
        for (callbacks = registry->callbacks; callbacks; callbacks = callbacks->hh.next) { // TODO: remove when UThash problem is fixed
            if (strcmp(callbacks->name, ev->name) == 0) {
                break;
            }
        }
        if (!callbacks) { // no handlers
            registry = registry->parent;
            continue; //return;
        }
        for (i = 0; i < callbacks->arr.length; i++) { // call the callbacks
            // TODO: dispatch to worker threads
            if (callbacks->arr.data[i].callback) {
                printf("func<%p> registry<%p> name<%s> size<%u> data<%p> ctx<%p>\n", callbacks->arr.data[i].callback, registry, ev->name, ev->size, &ev->data, callbacks->arr.data[i].ctx);
                callbacks->arr.data[i].callback(registry, ev->name, ev->size, &ev->data, callbacks->arr.data[i].ctx);
            }
        }
        registry = registry->parent;
    }
    if (!(ev->flags & LIFETIME_INFINITE)) {
        free(ev->name); // one off event, don't leak
        free(ev);
        free(ctx);
    }
}

ilE_registry* ilE_registry_new()
{
    ilE_registry* q = calloc(1, sizeof(ilE_registry));
    return q;
}

void ilE_registry_forward(ilE_registry *from, ilE_registry *to)
{
    from->parent = to;
}

static int push(ilE_registry* registry, const char *name, size_t size, const void *data)
{
    struct ilE_event *event = calloc(1, sizeof(struct ilE_event));
    struct dispatch_ctx * ctx = calloc(sizeof(struct dispatch_ctx), 1);
    event->name = strdup(name);
    if (size > 255) {
        return 0;
    }
    event->size = size;
    memcpy(&event->data, data, size);
    ctx->ev = event;
    ctx->registry = registry;
    struct event * ev = event_new(ilE_base, -1, 0, &dispatch, ctx);
    int res = event_add(ev, NULL);
    if (res != 0) return 0;
    event_active(ev, 0, 0);
    return 1;
}

static int timer(ilE_registry* registry, const char *name, size_t size, const void *data, struct timeval interval)
{
    int res;
    struct event * ev;
    struct ilE_event *event = calloc(1, sizeof(struct ilE_event));
    event->name = strdup(name);
    if (size > 255) {
        return 0;
    }
    event->size = size;
    memcpy(&event->data, data, size);
    event->flags = LIFETIME_INFINITE;
    struct dispatch_ctx * ctx = calloc(sizeof(struct dispatch_ctx), 1);
    ctx->ev = event;
    ctx->registry = registry;
    ev = event_new(ilE_base, -1, EV_PERSIST, &dispatch, ctx);
    struct timeval *tv = calloc(1, sizeof(struct timeval));
    tv->tv_sec = interval.tv_sec;
    tv->tv_usec = interval.tv_usec;
    res = event_add(ev, tv);
    if (res != 0) {
        return 0;
    }
    event_active(ev, 0, 0);

    return 1;
}

void ilE_globalevent(ilE_registry* registry, const char *name, size_t size, const void *data)
{
    if (!registry || !name) {
        return;
    }
    push(registry, name, size, data);
}

void ilE_typeevent(il_type* type, const char *name, size_t size, const void *data)
{
    if (!type || !type->registry || !name) { 
        return;
    }
    push(type->registry, name, size, data);
}

void ilE_objectevent(il_base* base, const char *name, size_t size, const void *data)
{
    if (!base || !base->registry || !name) {
        return;
    }
    push(base->registry, name, size, data);
}

void ilE_globaltimer(ilE_registry* registry, const char *name, size_t size, const void *data, struct timeval tv)
{
    if (!registry || !name) {
        return;
    }
    timer(registry, name, size, data, tv);
}

void ilE_typetimer(il_type* type, const char *name, size_t size, const void *data, struct timeval tv)
{
    if (!type || !type->registry || !name) {
        return;
    }
    timer(type->registry, name, size, data, tv);
}

void ilE_objecttimer(il_base* base, const char *name, size_t size, const void *data, struct timeval tv)
{
    if (!base || !base->registry || !name) {
        return;
    }
    timer(base->registry, name, size, data, tv);
}


int ilE_register(ilE_registry* registry, const char *name, enum ilE_behaviour behaviour, enum ilE_threading threads, ilE_callback callback, void * ctx)
{
    struct callbacks* callbacks = NULL;
    struct callback cb = {callback, behaviour, threads, ctx};

    //HASH_FIND_STR(registry->callbacks, name, callbacks);
    for (callbacks = registry->callbacks; callbacks; callbacks = callbacks->hh.next) { // TODO: remove when UThash problem is fixed
        if (strcmp(callbacks->name, name) == 0) {
            break;
        }
    }

    if (callbacks == NULL) {
        callbacks = calloc(1, sizeof(struct callbacks));
        callbacks->name = strdup(name);
        HASH_ADD_STR(registry->callbacks, name, callbacks);
    }

    if (behaviour == ILE_OVERRIDE) {
        callbacks->arr.length = 0;
    }

    if (behaviour == ILE_BEFORE) {
        if (callbacks->arr.length < 1) {
            IL_SET(callbacks->arr, 0, cb); // first
        } else {
            if (callbacks->arr.data[0].behaviour == ILE_DONTCARE) {
                // move the first element to the last spot so we can get at it
                IL_APPEND(callbacks->arr, callbacks->arr.data[0]);
            } else {
                // we shift the entire list to the right
                struct callback * unused;
                IL_INDEXORZERO(callbacks->arr, callbacks->arr.length, unused);
                memmove(&callbacks->arr.data[1], &callbacks->arr.data[0], sizeof(struct callback) * (callbacks->arr.length-1));
                IL_SET(callbacks->arr, 0, cb);
            }
        }
    } else {
        // can be appended
        IL_APPEND(callbacks->arr, cb);
    }

    return 0;
}

void shutdown_callback(const ilE_registry* registry, const char *name, size_t size, const void *data, void * ctx)
{
    (void)registry, (void)name, (void)size, (void)data, (void)ctx;
    il_log("Shutting down.");
    event_base_loopbreak(ilE_base);
}

void ilE_init()
{
    ilE_base = event_base_new();
    il_registry = ilE_registry_new();
    struct timeval tv = (struct timeval) {
        0, IL_BASE_TICK_LENGTH
    };
    ilE_globaltimer(il_registry, "tick", 0, NULL, tv);
    ilE_register(il_registry, "shutdown", ILE_BEFORE, ILE_ANY, &shutdown_callback, NULL);
}

void ilE_loop()
{
    event_base_loop(ilE_base, 0);
}

void ilE_dumpHooks(ilE_registry *registry)
{
    struct callbacks *callbacks;
    int i;
    fprintf(stderr, "Registry dump of registry %p:\n", registry);
    for (callbacks = registry->callbacks; callbacks != NULL; callbacks = callbacks->hh.next) {
        fprintf(stderr, "Callbacks for event %s:\n", callbacks->name);
        for (i = 0; i < callbacks->arr.length; i++) {
            struct callback *cb = &callbacks->arr.data[i];
            fprintf(stderr, "\tfunc<%p> behaviour:%u threading:%u ctx<%p>\n", cb->callback, cb->behaviour, cb->threading, cb->ctx);
        }
    }
    fprintf(stderr, "End of registry dump.\n");
}

