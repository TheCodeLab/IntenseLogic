#ifndef COMMON_EVENT_H
#define COMMON_EVENT_H

#include <stdint.h>
#include <stddef.h>

struct timeval;

typedef struct il_Event_Event {
  uint16_t eventid;
  uint8_t size;
  uint8_t data[];
} il_Event_Event;

typedef void(*il_Event_Callback)(il_Event_Event*, void * ctx);

const il_Event_Event* il_Event_new(uint16_t eventid, uint8_t size, void * data);

int il_Event_push(const il_Event_Event* event);

int il_Event_pushnew(uint16_t eventid, uint8_t size, void * data);

int il_Event_timer(const il_Event_Event* event, struct timeval * interval);

int il_Event_register(uint16_t eventid, il_Event_Callback callback, void * ctx);

#endif
