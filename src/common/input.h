#ifndef COMMON_INPUT_H
#define COMMON_INPUT_H

#include <SDL/events.h>

#include "event.h"

#define IL_INPUT_EID 1 << 8

enum {
  IL_INPUT_SDL_EVENT = IL_INPUT_EID
}

typedef struct il_Input_SdlEvent {
  il_Event_Event parent;
  SDL_Event event;
} il_Input_SdlEvent;

#endif
