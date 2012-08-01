#ifndef COMMON_EVENT_H
#define COMMON_EVENT_H

#include <stdint.h>
#include <stddef.h>

typedef struct il_Event_Event {
  uint16_t eventid;
  uint8_t size;
  uint8_t data[];
} il_Event_Event;

typedef struct il_Event_Node {
  il_Event_Event* event;
  struct il_Event_Node *next;
} il_Event_Node;

il_Event_Node *il_Event_EventQueue_first;
il_Event_Node *il_Event_EventQueue_last;

void il_Event_push(const il_Event_Event* event);

const il_Event_Event* il_Event_pop();


typedef void(*il_Event_Callback)(il_Event_Event*);

typedef struct il_Event_CallbackContainer {
  uint16_t eventid;
  size_t length;
  il_Event_Callback* callbacks;
} il_Event_CallbackContainer;

il_Event_CallbackContainer *il_Event_Callbacks;
size_t il_Event_Callbacks_len;

void il_Event_handle(il_Event_Event* ev);

void il_Event_register(uint16_t eventid, il_Event_Callback callback);

#endif
