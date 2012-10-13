#ifndef SG_QUATERNION
#define SG_QUATERNION

typedef struct sg_Quaternion {
  float x, y, z, w;
} sg_Quaternion;

#endif

#ifndef SG_QUATERNION_H
#define SG_QUATERNION_H

#include "matrix.h"
#include "vector.h"

sg_Quaternion sg_Quaternion_fromAxisAngle(sg_Vector3 v, float a);
sg_Quaternion sg_Quaternion_fromYPR(float bank, float heading, float attitutde);
sg_Quaternion sg_Quaternion_mul(sg_Quaternion a, sg_Quaternion b);

#endif
