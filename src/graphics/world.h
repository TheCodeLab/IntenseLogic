#ifndef IL_GRAPHICS_WORLD_H
#define IL_GRAPHICS_WORLD_H

#include "common/world.h"
#include "graphics/camera.h"

typedef struct ilG_world {
  il_world *world;
  unsigned refs;
  ilG_camera *camera;
} ilG_world;

ilG_world *ilG_active_world;

ilG_world * ilG_world_new();
ilG_world * ilG_world_new_world(il_world * world);

ilG_world * il_world_getGraphicsWorld(il_world * world);

#endif
