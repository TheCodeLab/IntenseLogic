#ifndef IL_PHYSICS_PHYSICS_H
#define IL_PHYSICS_PHYSICS_H

#include "world.h"

#define IL_PHYSICS_RANGE  (3)

#define IL_PHYSICS_STEPSPERTICK (1)

void ilP_init();

void ilP_registerWorld(ilP_world *world);

#endif
