#ifndef COMMON_EVENT_H
#define COMMON_EVENT_H

#include <stdint.h>
#include <stddef.h>
#include <event2/event.h>

struct event_base * ilE_base;

struct timeval;

typedef struct ilE_event {
  uint16_t eventid;
  uint8_t size;
  uint8_t data[];
} ilE_event;

typedef void(*ilE_callback)(ilE_event*, void * ctx);

const ilE_event* ilE_new(uint16_t eventid, uint8_t size, void * data);

int ilE_push(const ilE_event* event);

int ilE_pushnew(uint16_t eventid, uint8_t size, void * data);

int ilE_timer(const ilE_event* event, struct timeval * interval);

int ilE_register(uint16_t eventid, ilE_callback callback, void * ctx);

#endif
