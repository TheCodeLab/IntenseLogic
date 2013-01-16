#include "event.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <event2/event.h>

#include "common/base.h"
#include "common/log.h"
#include "common/array.h"

enum flags {
    LIFETIME_INFINITE = 1
};

struct ilE_event {
    uint16_t id;
    uint8_t size;
    uint8_t flags;
    char data[];
};

struct callback {
    ilE_callback callback;
    enum ilE_behaviour behaviour;
    void * ctx;
};

IL_ARRAY(struct callback, callback_array);

struct callbackContainer {
    uint16_t id;
    callback_array arr;
};

IL_ARRAY(struct callbackContainer, callbackContainer_array);

struct ilE_queue {
    struct event_base * base;
    callbackContainer_array callbacks;
};

struct dispatch_ctx {
    ilE_event* ev;
    ilE_queue* queue;
};

static void dispatch(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd;
    (void)events;
    ilE_event* ev = ((struct dispatch_ctx*)ctx)->ev;
    ilE_queue* queue = ((struct dispatch_ctx*)ctx)->queue;
    unsigned i;
    // figure out which entry in our callbacks array array is for this event
    struct callbackContainer* container = NULL;
    for (i = 0; i < queue->callbacks.length; i++) {
        if (queue->callbacks.data[i].id == ev->id) {
            container = &queue->callbacks.data[i];
            break;
        }
    }

    if (!container) { // no handlers
        return;
    }

    for (i = 0; i < container->arr.length; i++) { // call the callbacks
        // TODO: dispatch to worker threads
        if (container->arr.data[i].callback) {
            container->arr.data[i].callback(queue, ev, container->arr.data[i].ctx);
        }
    }

    if (!(ev->flags & LIFETIME_INFINITE)) {
        free(ev); // one off event, don't leak
    }
}

ilE_queue* ilE_queue_new()
{
    ilE_queue* q = calloc(sizeof(ilE_queue), 1);
    q->base = event_base_new();
    return q;
}

ilE_event* ilE_new(uint16_t eventid, uint8_t size, void * data)
{
    ilE_event * ev = calloc(1, sizeof(ilE_event) + size);
    ev->id = eventid;
    ev->size = size;
    if (data)
        memcpy(&ev->data, data, size);
    return ev;
}

void * ilE_getData(const ilE_event* event, size_t *size)
{
    if (size) {
        *size = event->size;
    }
    return &event->data;
}

int ilE_push(ilE_queue* queue, ilE_event* event)
{
    if (event == NULL) return -1;
    struct dispatch_ctx * ctx = calloc(sizeof(struct dispatch_ctx), 1);
    ctx->ev = event;
    ctx->queue = queue;
    struct event * ev = event_new(queue->base, -1, 0, 
        &dispatch, ctx);
    int res = event_add(ev, NULL);
    if (res != 0) return -1;
    event_active(ev, 0, 0);
    return 0;
}

int ilE_timer(ilE_queue* queue, ilE_event* event, struct timeval * interval)
{
    int res;
    struct event * ev;

    struct dispatch_ctx * ctx = calloc(sizeof(struct dispatch_ctx), 1);
    ctx->ev = event;
    ctx->queue = queue;

    event->flags |= LIFETIME_INFINITE;
    ev = event_new(queue->base, -1, EV_TIMEOUT|EV_PERSIST, &dispatch, ctx);
    res = event_add(ev, interval);
    if (res != 0) return -1;
    event_active(ev, 0, 0);

    return 0;
}

int ilE_register(ilE_queue* queue, uint16_t eventid, enum ilE_behaviour behaviour, ilE_callback callback, void * ctx)
{
    unsigned i;
    struct callbackContainer* container = NULL;
    struct callback cb = {callback, behaviour, ctx};

    for (i = 0; i < queue->callbacks.length; i++) {
        if (queue->callbacks.data[i].id == eventid) {
            container = &queue->callbacks.data[i];
            break;
        }
    }

    if (container == NULL) {
        IL_INDEXORZERO(queue->callbacks, queue->callbacks.length, container);
        container->id = eventid;
    }

    if (behaviour == ILE_OVERRIDE) {
        container->arr.length = 0;
    }

    if (behaviour == ILE_BEFORE) {
        if (container->arr.length < 1) {
            IL_SET(container->arr, 0, cb); // FIRST!!11!!!!1
        } else {
            if (container->arr.data[0].behaviour == ILE_DONTCARE) {
                // move the first element to the last spot so we can get at it
                IL_APPEND(container->arr, container->arr.data[0]);
            } else {
                // we shift the entire list to the right
                struct callback * unused;
                IL_INDEXORZERO(container->arr, container->arr.length, unused);
                memmove(&container->arr.data[1], &container->arr.data[0], sizeof(struct callback) * (container->arr.length-1));
                IL_SET(container->arr, 0, cb);
            }
        }
    } else {
        // can be appended
        IL_APPEND(container->arr, cb);
    }

    return 0;
}

void shutdown_callback(ilE_event* ev)
{
    (void)ev;
    il_log(3, "Shutting down.");
    event_base_loopbreak(il_queue->base);
}

void ilE_init()
{
    il_queue = ilE_queue_new();

    ilE_event * tick = ilE_new(IL_BASE_TICK, 0, NULL);
    struct timeval * tv = calloc(1, sizeof(struct timeval));
    *tv = (struct timeval) {
        0, IL_BASE_TICK_LENGTH
    };
    ilE_timer(il_queue, tick, tv);

    ilE_register(il_queue, IL_BASE_SHUTDOWN, ILE_BEFORE, (ilE_callback)&shutdown_callback, NULL);
}

void ilE_loop()
{
    event_base_loop(il_queue->base, 0);
}

