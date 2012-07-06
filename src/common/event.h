#ifndef COMMON_EVENT_H
#define COMMON_EVENT_H

typedef struct il_Event_Event {
  unsigned short eventid;
  unsigned char size;
  //void data[];
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
  unsigned short eventid;
  unsigned short length;
  il_Event_Callback* callbacks;
} il_Event_CallbackContainer;

il_Event_CallbackContainer *il_Event_Callbacks;
unsigned il_Event_Callbacks_len;

void il_Event_handle(il_Event_Event* ev);

void il_Event_register(unsigned short eventid, il_Event_Callback callback);

#endif
