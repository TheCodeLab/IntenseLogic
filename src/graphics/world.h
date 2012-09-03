#ifndef IL_GRAPHICS_WORLD_H
#define IL_GRAPHICS_WORLD_H

#include "common/world.h"
#include "graphics/camera.h"

typedef struct il_Graphics_World {
  il_Common_World *world;
  unsigned refs;
  il_Graphics_Camera *camera;
} il_Graphics_World;

il_Graphics_World *active;

#endif
