#include "event.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <event2/event.h>

#include "common/base.h"
#include "common/log.h"

struct event_base * ilE_base;

struct callback {
    ilE_callback callback;
    void * ctx;
};

typedef struct ilE_callbackContainer {
    uint16_t eventid;
    size_t length;
    struct callback *callbacks;
} ilE_callbackContainer;

ilE_callbackContainer *ilE_callbacks;
size_t ilE_callbacks_len;

void ilE_dispatch(evutil_socket_t fd, short events, ilE_event * ev)
{
    (void)fd;
    (void)events;
    /*il_log(5, "Event %i dispatched, %i bytes of data\n", (int)ev->eventid,
      (int)ev->size);*/
    unsigned i;
    struct ilE_callbackContainer* container = NULL;
    for (i = 0; i < ilE_callbacks_len; i++) {
        if (ilE_callbacks[i].eventid == ev->eventid) {
            container = &ilE_callbacks[i];
            break;
        }
    }

    if (!container || !container->callbacks) {
        return;
    }

    for (i = 0; i < container->length; i++) {
        container->callbacks[i].callback(ev, container->callbacks[i].ctx);
    }

    // TODO: find a way to get rid of this memory leak
    /*if (!(events & EV_PERSIST))
      free(ev);*/
}

const ilE_event* ilE_new(uint16_t eventid, uint8_t size, void * data)
{
    ilE_event * ev = calloc(1, sizeof(ilE_event) + size);
    ev->eventid = eventid;
    ev->size = size;
    if (data)
        memcpy(&ev->data, data, size);
    return ev;
}

int ilE_push(const ilE_event* event)
{
    if (event == NULL) return -1;
    struct event * ev = event_new(ilE_base, -1, 0,
                                  (event_callback_fn)&ilE_dispatch, (void*)event);
    int res = event_add(ev, NULL);
    if (res != 0) return -1;
    event_active(ev, 0, 0);
    return 0;
}

int ilE_pushnew(uint16_t eventid, uint8_t size, void * data)
{
    return ilE_push(ilE_new(eventid, size, data));
}

int ilE_timer(const ilE_event* event, struct timeval * interval)
{
    int res;
    struct event * ev = event_new( ilE_base, -1, EV_TIMEOUT|EV_PERSIST,
                                   (event_callback_fn)&ilE_dispatch, (void*)event);
    res = event_add(ev, interval);
    if (res != 0) return -1;
    event_active(ev, 0, 0);

    return 0;
}

int ilE_register(uint16_t eventid, ilE_callback callback, void * ctx)
{
    unsigned i;
    struct ilE_callbackContainer* container = NULL;
    int append = 0;
    for (i = 0; i < ilE_callbacks_len; i++) {
        if (ilE_callbacks[i].eventid == eventid) {
            container = &ilE_callbacks[i];
            break;
        }
    }

    if (container == NULL) {
        container = calloc(1, sizeof(struct ilE_callbackContainer));
        container->eventid = eventid;
        container->length = 0;
        container->callbacks = NULL;
        append = 1;
    }

    struct callback* temp = (struct callback*)calloc((container->length+1), sizeof(struct callback));
    memcpy(temp, container->callbacks, sizeof(struct callback) * container->length);
    free(container->callbacks);
    temp[container->length] = (struct callback) {
        callback, ctx
    };
    container->length++;
    container->callbacks = temp;

    if (append) {
        ilE_callbackContainer *temp2 = (ilE_callbackContainer*)calloc((ilE_callbacks_len+1), sizeof(ilE_callbackContainer));
        memcpy(temp2, ilE_callbacks, sizeof(ilE_callbackContainer) * ilE_callbacks_len);
        temp2[ilE_callbacks_len] = *container;
        free(ilE_callbacks);
        ilE_callbacks = temp2;
        ilE_callbacks_len++;
    }

    return 0;
}

void ilE_init()
{
    ilE_base = event_base_new();

    const ilE_event * tick = ilE_new(IL_BASE_TICK, 0, NULL);
    struct timeval * tv = calloc(1, sizeof(struct timeval));
    *tv = (struct timeval) {
        0, IL_BASE_TICK_LENGTH
    };
    ilE_timer(tick, tv);
}
