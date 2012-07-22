#ifndef IL_PHYSICS_WORLD_H
#define IL_PHYSICS_WORLD_H

#include <ode/ode.h>

#include "common/world.h"

typedef struct il_Physics_World {
  il_Common_World *world;
  float gravity;
  unsigned refs;
  dWorldID worldid;
  int active;
} il_Physics_World;

il_Physics_World* il_Physics_World_create(const il_Common_World* parent);

#endif
