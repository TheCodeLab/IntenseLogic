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

#include "quaternion.h"

#include <math.h>

sg_Quaternion sg_Quaternion_fromAxisAngle(sg_Vector3 v, float a) {
  sg_Quaternion q;
  float s = sinf(a/2);
  q.x = s * v.x;
  q.y = s * v.y;
  q.z = s * v.z;
  q.w = cosf(a/2);
  return q;
}

sg_Quaternion sg_Quaternion_fromYPR(float bank, float heading, float attitude) {
  sg_Quaternion quat;
  float c1 = cos(heading/2);
  float c2 = cos(attitude/2);
  float c3 = cos(bank/2);
  float s1 = sin(heading/2);
  float s2 = sin(attitude/2);
  float s3 = sin(bank/2);
  
  quat.w = (c1 * c2 * c3) - (s1 * s2 * s3);
  quat.x = (s1 * s2 * c3) + (c1 * c2 * s3);
  quat.y = (s1 * c2 * c3) + (c1 * s2 * s3);
  quat.z = (c1 * s2 * c3) - (s1 * c2 * s3);
  
  return quat;
}

// ab = (a1b1 − a2b2 − a3b3 − a4b4) + (a1b2 + a2b1 + a3b4 − a4b3)i + 
// (a1b3 − a2b4 + a3b1 + a4b2)j + (a1b4 + a2b3 − a3b2 + a4b1)k 
// http://content.gpwiki.org/index.php/Quaternions
sg_Quaternion sg_Quaternion_mul(sg_Quaternion a, sg_Quaternion b) {
  sg_Quaternion c;
  
  float a1 = a.w;
  float a2 = a.x;
  float a3 = a.y;
  float a4 = a.z;
  
  float b1 = b.w;
  float b2 = b.x;
  float b3 = b.y;
  float b4 = b.z;
  
  c.w = a1*b1 - a2*b2 - a3*b3 - a4*b4;
  c.x = a1*b2 + a2*b1 + a3*b4 - a4*b3;
  c.y = a1*b3 - a2*b4 + a3*b1 + a4*b2;
  c.z = a1*b4 + a2*b3 - a3*b2 + a4*b1;
  
  return c;
}
