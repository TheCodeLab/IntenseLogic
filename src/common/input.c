#include "input.h"

#include <string.h>
#include <stdlib.h>

#include "common/log.h"
#include "common/event.h"

static int keys_set[16];
static int buttons_set[16];

static void set(il_Event_Event* ev, void * ctx) {
  int * arr = (int*)ctx;
  int i = 0;
  while (i < 16 && arr[i]) i++;
  if (i >= 16) {
    il_Common_log(2, "Too many inputs set! (16)");
    return;
  }
  arr[i] = *(int*)&ev->data;
  il_Common_log(4, "Pressed key %i, assigned slot %i\n", arr[i], i);
  /*int test = il_Input_isKeySet(arr[i]);
  il_Common_log(4, "Test key on/off: %i\n", test);*/
}

static void unset(il_Event_Event* ev, void * ctx) {
  int * arr = (int*)ctx;
  int key = *(int*)&ev->data;
  int i = 0;
  while (i < 16 && arr[i] != key) i++;
  if (i >= 16) {
    il_Common_log(2, "Too many inputs set! (16)");
    return;
  }
  arr[i] = 0;
  il_Common_log(4, "Unset key %i at slot %i\n", key, i);
}

void il_Input_init() {
  il_Common_log(4, "\n");
  memset(keys_set, 0, sizeof(int) * 16);
  memset(buttons_set, 0, sizeof(int) * 16);
  
  il_Event_register(IL_INPUT_KEYDOWN, (il_Event_Callback)&set, &keys_set);
  il_Event_register(IL_INPUT_KEYUP, (il_Event_Callback)&unset, &keys_set);
  
  il_Event_register(IL_INPUT_MOUSEDOWN, (il_Event_Callback)&set, &buttons_set);
  il_Event_register(IL_INPUT_MOUSEUP, (il_Event_Callback)&unset, &buttons_set);
}

int il_Input_isKeySet(int key) {
  int i = 0;
  while (i < 16 && keys_set[i] != key) i++;
  return i != 16;
}

int il_Input_isButtonSet(int button) {
  int i = 0;
  while (i < 16 && buttons_set[i] != button) i++;
  return i != 16;
}
