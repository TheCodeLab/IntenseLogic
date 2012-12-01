#ifndef IL_PHYSICS_JOINT_H
#define IL_PHYSICS_JOINT_H

#include <ode/ode.h>

#include "world.h"

typedef struct ilP_joint {
  unsigned refs;
  dJointID jointid;
  ilP_world *world;
} ilP_joint;

#endif
