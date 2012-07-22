#ifndef IL_PHYSICS_RIGID_H
#define IL_PHYSICS_RIGID_H

#include <ode/ode.h>

#include "common/positionable.h"

typedef struct il_Physics_Rigid {
  unsigned refs;
  il_Common_Positionable *positionable;
  float mass;
  dBodyID bodyid;
  dGeomID geomid;
} il_Physics_Rigid;

#endif
