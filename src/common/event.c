#include "event.h"

#include <stdlib.h>
#include <string.h>

void il_Event_push(const il_Event_Event* event) {
  struct il_Event_Node *node = malloc(sizeof(struct il_Event_Node));
  
  node->event = (il_Event_Event*)event;
  node->next = NULL;
  
  if (il_Event_EventQueue_last != NULL)
    il_Event_EventQueue_last->next = node;
  il_Event_EventQueue_last = node;
}

const il_Event_Event* il_Event_pop() {
  struct il_Event_Node *node = il_Event_EventQueue_first;
  if (node == NULL) {
    return NULL;
  }
  if (node == il_Event_EventQueue_last) {
    il_Event_EventQueue_last = NULL;
  }
  il_Event_EventQueue_first = node->next;
  
  return node->event;
}

void il_Event_handle(il_Event_Event* ev) {
  int i;
  struct il_Event_CallbackContainer* container;
  for (i = 0; i < il_Event_Callbacks_len; i++) {
    if (il_Event_Callbacks[i].eventid == ev->eventid) {
      container = &il_Event_Callbacks[i];
      break;
    }
  }
  
  if (!container) {
    return;
  }
  
  for (i = 0; i < container->length; i++) {
    container->callbacks[i](ev);
  }
  
  free(ev);
  
}

void il_Event_register(unsigned short eventid, il_Event_Callback callback) {
  int i;
  struct il_Event_CallbackContainer* container;
  for (i = 0; i < il_Event_Callbacks_len; i++) {
    if (il_Event_Callbacks[i].eventid == eventid) {
      container = &il_Event_Callbacks[i];
      break;
    }
  }
  
  if (!container) {
    container = malloc(sizeof(struct il_Event_CallbackContainer));
    container->eventid = eventid;
    container->length = 0;
    container->callbacks = NULL;
  }
  
  il_Event_Callback* temp = (il_Event_Callback*)realloc(container->callbacks, container->length);
  memcpy(temp, container->callbacks, container->length);
  temp[container->length] = callback;
  container->length++;
  container->callbacks = temp;
  
}
