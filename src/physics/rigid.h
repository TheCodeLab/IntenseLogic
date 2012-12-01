#ifndef IL_PHYSICS_RIGID_H
#define IL_PHYSICS_RIGID_H

#include <ode/ode.h>

#include "common/positionable.h"

typedef struct ilP_rigid {
  unsigned refs;
  il_positionable *positionable;
  float mass;
  dBodyID bodyid;
  dGeomID geomid;
} ilP_rigid;

#endif
