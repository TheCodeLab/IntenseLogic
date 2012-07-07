#ifndef IL_COMMON_POSITIONABLE_H
#define IL_COMMON_POSITIONABLE_H

#include "matrix.h"

typedef struct il_Common_Positionable {
  sg_Vector3 position;
  sg_Vector3 velocity;
  sg_Matrix rotation;
  sg_Matrix rotvelocity;
  sg_Vector3 size;
} il_Common_Positionable;

#endif
