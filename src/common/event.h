#ifndef COMMON_EVENT_H
#define COMMON_EVENT_H

#include <stdint.h>
#include <stddef.h>

#define ilE_pushnew(q, e, s, d) ilE_push(q, ilE_new(e,s,d))

struct timeval;

typedef struct ilE_queue ilE_queue;

typedef struct ilE_event ilE_event;

enum ilE_behaviour {
    ILE_DONTCARE,
    ILE_BEFORE,
    ILE_AFTER,
    ILE_OVERRIDE
};

ilE_queue* il_queue;

typedef void(*ilE_callback)(const ilE_queue*, const ilE_event*, void * ctx);

ilE_queue* ilE_queue_new();

ilE_event* ilE_new(uint16_t eventid, uint8_t size, void * data);

uint16_t ilE_getID(const ilE_event* event);

void * ilE_getData(const ilE_event* event, size_t *size);

int ilE_push(ilE_queue* queue, ilE_event* event);

int ilE_timer(ilE_queue* queue, ilE_event* event, struct timeval * interval);

int ilE_register(ilE_queue* queue, uint16_t eventid, enum ilE_behaviour behaviour, ilE_callback callback, void * ctx);

#endif

