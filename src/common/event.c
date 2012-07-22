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
  if (il_Event_EventQueue_first == NULL)
    il_Event_EventQueue_first = node;
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
    container->callbacks[i](ev);
  }
  
  free(ev);
  
}

void il_Event_register(unsigned short eventid, il_Event_Callback callback) {
  int i;
  struct il_Event_CallbackContainer* container = NULL;
  for (i = 0; i < il_Event_Callbacks_len; i++) {
    if (il_Event_Callbacks[i].eventid == eventid) {
      container = &il_Event_Callbacks[i];
      break;
    }
  }
  
  if (container == NULL) {
    container = malloc(sizeof(struct il_Event_CallbackContainer));
    container->eventid = eventid;
    container->length = 0;
    container->callbacks = NULL;
  }
  
  il_Event_Callback* temp = (il_Event_Callback*)malloc(sizeof(il_Event_Callback) * (container->length+1));
  memcpy(temp, container->callbacks, sizeof(il_Event_Callback) * container->length);
  //if (container->callbacks != NULL)
  free(container->callbacks);
  temp[container->length] = callback;
  container->length++;
  container->callbacks = temp;
  
  il_Event_CallbackContainer *temp2 = (il_Event_CallbackContainer*)malloc(sizeof(il_Event_CallbackContainer) * il_Event_Callbacks_len);
  memcpy(temp2, il_Event_Callbacks, il_Event_Callbacks_len);
  temp2[il_Event_Callbacks_len] = *container;
  free(il_Event_Callbacks);
  il_Event_Callbacks = temp2;
  il_Event_Callbacks_len++;
  
}
