#ifndef IL_COMMON_POSITIONABLE_H
#define IL_COMMON_POSITIONABLE_H

#include "matrix.h"
#include "world.h"

typedef struct il_Common_Positionable {
  sg_Vector3 position;
  sg_Matrix rotation;
  sg_Vector3 size;
  il_Common_World *parent;
  unsigned refs;
} il_Common_Positionable;

#endif
