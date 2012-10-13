#include "event.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <event2/event.h>

#include "common/base.h"
#include "common/log.h"

struct event_base * il_Event_base;

struct callback {
  il_Event_Callback callback;
  void * ctx;
};

typedef struct il_Event_CallbackContainer {
  uint16_t eventid;
  size_t length;
  struct callback *callbacks;
} il_Event_CallbackContainer;

il_Event_CallbackContainer *il_Event_Callbacks;
size_t il_Event_Callbacks_len;

void il_Event_dispatch(evutil_socket_t fd, short events, il_Event_Event * ev) {
  /*il_Common_log(5, "Event %i dispatched, %i bytes of data\n", (int)ev->eventid,
    (int)ev->size);*/
  int i;
  struct il_Event_CallbackContainer* container = NULL;
  for (i = 0; i < il_Event_Callbacks_len; i++) {
    if (il_Event_Callbacks[i].eventid == ev->eventid) {
      container = &il_Event_Callbacks[i];
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

const il_Event_Event* il_Event_new(uint16_t eventid, uint8_t size, void * data) {
  il_Event_Event * ev = calloc(1, sizeof(il_Event_Event) + size);
  ev->eventid = eventid;
  ev->size = size;
  if (data)
    memcpy(&ev->data, data, size);
  return ev;
}

int il_Event_push(const il_Event_Event* event) {
  if (event == NULL) return -1;
  struct event * ev = event_new(il_Event_base, -1, 0, 
    (event_callback_fn)&il_Event_dispatch, (void*)event);
  int res = event_add(ev, NULL);
  if (res != 0) return -1;
  event_active(ev, 0, 0);
  return 0;
}

int il_Event_pushnew(uint16_t eventid, uint8_t size, void * data) {
  return il_Event_push(il_Event_new(eventid, size, data));
}

int il_Event_timer(const il_Event_Event* event, struct timeval * interval) {
  int res;
  struct event * ev = event_new( il_Event_base, -1, EV_TIMEOUT|EV_PERSIST, 
    (event_callback_fn)&il_Event_dispatch, (void*)event);
  res = event_add(ev, interval);
  if (res != 0) return -1;
  event_active(ev, 0, 0);
  
  return 0;
}

int il_Event_register(uint16_t eventid, il_Event_Callback callback, void * ctx) {
  int i;
  struct il_Event_CallbackContainer* container = NULL;
  int append = 0;
  for (i = 0; i < il_Event_Callbacks_len; i++) {
    if (il_Event_Callbacks[i].eventid == eventid) {
      container = &il_Event_Callbacks[i];
      break;
    }
  }
  
  if (container == NULL) {
    container = calloc(1, sizeof(struct il_Event_CallbackContainer));
    container->eventid = eventid;
    container->length = 0;
    container->callbacks = NULL;
    append = 1;
  }
  
  struct callback* temp = (struct callback*)calloc((container->length+1), sizeof(struct callback));
  memcpy(temp, container->callbacks, sizeof(struct callback) * container->length);
  free(container->callbacks);
  temp[container->length] = (struct callback){callback, ctx};
  container->length++;
  container->callbacks = temp;
  
  if (append) {
    il_Event_CallbackContainer *temp2 = (il_Event_CallbackContainer*)calloc((il_Event_Callbacks_len+1), sizeof(il_Event_CallbackContainer));
    memcpy(temp2, il_Event_Callbacks, sizeof(il_Event_CallbackContainer) * il_Event_Callbacks_len);
    temp2[il_Event_Callbacks_len] = *container;
    free(il_Event_Callbacks);
    il_Event_Callbacks = temp2;
    il_Event_Callbacks_len++;
  }
  
  return 0;
}

void il_Event_init() {
  il_Event_base = event_base_new();
  
  const il_Event_Event * tick = il_Event_new(IL_BASE_TICK, 0, NULL);
  struct timeval * tv = calloc(1, sizeof(struct timeval));
  *tv = (struct timeval){0, IL_BASE_TICK_LENGTH};
  il_Event_timer(tick, tv);
}
