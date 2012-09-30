// Copyright (c) 2012 Code Lab
//
// This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
//
//     2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
//
//     3. This notice may not be removed or altered from any source distribution.
//

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
