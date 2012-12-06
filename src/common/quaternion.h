#ifndef SG_QUATERNION
#define SG_QUATERNION

typedef struct il_Quaternion {
  float x, y, z, w;
} il_Quaternion;

#endif

#ifndef SG_QUATERNION_H
#define SG_QUATERNION_H

#include "matrix.h"
#include "vector.h"

il_Quaternion il_Quaternion_fromAxisAngle(il_Vector3 v, float a);
il_Quaternion il_Quaternion_fromYPR(float bank, float heading, float attitutde);
il_Quaternion il_Quaternion_mul(il_Quaternion a, il_Quaternion b);

#endif
