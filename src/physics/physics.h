#ifndef IL_PHYSICS_PHYSICS_H
#define IL_PHYSICS_PHYSICS_H

#include "world.h"

#define IL_PHYSICS_RANGE  (3)

#define IL_PHYSICS_STEPSPERTICK (1)

void il_Physics_init();

void il_Physics_registerWorld(il_Physics_World *world);

#endif
