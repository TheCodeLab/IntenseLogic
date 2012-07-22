#ifndef IL_PHYSICS_JOINT_H
#define IL_PHYSICS_JOINT_H

#include <ode/ode.h>

#include "world.h"

typedef struct il_Physics_Joint {
  unsigned refs;
  dJointID jointid;
  il_Physics_World *world;
} il_Physics_Joint;

#endif
