#ifndef IL_PHYSICS_WORLD_H
#define IL_PHYSICS_WORLD_H

#include <ode/ode.h>

#include "common/world.h"

typedef struct ilP_world {
  il_world *world;
  float gravity;
  unsigned refs;
  dWorldID worldid;
  int active;
} ilP_world;

ilP_world* ilP_world_create(const il_world* parent);

#endif
