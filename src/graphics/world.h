#ifndef IL_GRAPHICS_WORLD_H
#define IL_GRAPHICS_WORLD_H

#include "common/world.h"
#include "graphics/camera.h"

typedef struct il_Graphics_World {
  il_Common_World *world;
  unsigned refs;
  il_Graphics_Camera *camera;
} il_Graphics_World;

il_Graphics_World *il_Graphics_active_world;

il_Graphics_World * il_Graphics_World_new();
il_Graphics_World * il_Graphics_World_new_world(il_Common_World * world);

il_Graphics_World * il_Common_World_getGraphicsWorld(il_Common_World * world);

#endif
