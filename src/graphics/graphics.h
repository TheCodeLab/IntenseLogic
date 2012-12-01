#ifndef IL_GRAPHICS_GRAPHICS_H
#define IL_GRAPHICS_GRAPHICS_H

#include "common/event.h"

#define IL_GRAPHICS_RANGE   (2)
#define IL_GRAPHICS_TICK    ((IL_GRAPHICS_RANGE<<8) + 0)

#define IL_GRAPHICS_TICK_LENGTH (16666)

void ilG_init();

#endif
