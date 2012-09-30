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

#ifndef SG_VECTOR
#define SG_VECTOR

typedef struct sg_Vector2 {
  float x, y;
} sg_Vector2;

typedef struct sg_Vector3 {
  float x, y, z;
} sg_Vector3;

typedef struct sg_Vector4 {
  float x,y,z,w;
} sg_Vector4;

#endif

#ifndef SG_VECTOR_H
#define SG_VECTOR_H

#include "quaternion.h"

/* Vector naming convention

Vector functions have the following prototype:
ret vtype_op[_argtype] (vtype, ... [, argtype]);

*/

/* Typedef: sg_Vector2
A type used for representing 2-dimensional vectors.

Variables:
  x, y - The coordinates
*/

sg_Vector2 sg_Vector2_add(sg_Vector2 a, sg_Vector2 b);
sg_Vector2 sg_Vector2_add_f(sg_Vector2 a, float b);
sg_Vector2 sg_Vector2_sub(sg_Vector2 a, sg_Vector2 b);
sg_Vector2 sg_Vector2_sub_f(sg_Vector2 a, float b);
sg_Vector2 sg_Vector2_mul(sg_Vector2 a, sg_Vector2 b);
sg_Vector2 sg_Vector2_mul_f(sg_Vector2 a, float b);
sg_Vector2 sg_Vector2_div(sg_Vector2 a, sg_Vector2 b);
sg_Vector2 sg_Vector2_div_f(sg_Vector2 a, float b);

/* Function: sg_Vector2_len
Uses pythagorean theorem on the given vector, to return the number of units from
the point specified by it to the origin (0,0).
*/
float sg_Vector2_len(sg_Vector2 v);

/* Function: sg_Vector2_normalise
Returns a vector whose length is always 1, pointing in the same direction as the
given vector.
*/
sg_Vector2 sg_Vector2_normalise(sg_Vector2 v);

/* Function: sg_Vector2_dot
Returns the dot product of the two vectors.
*/
float sg_Vector2_dot(sg_Vector2 a, sg_Vector2 b);


/* Typedef: sg_Vector3
A type used for representing 3-dimensional vectors. 

Variables:
  x, y, z - The coordinates
*/

/* Function: sg_Vector3_add
Adds two vectors together using (x1+x2, y1+y2, z1+z2).
*/
sg_Vector3 sg_Vector3_add(sg_Vector3 a, sg_Vector3 b);

/* Function: sg_Vector3_sub
sg_Vector3_add except subtracting.
*/
sg_Vector3 sg_Vector3_sub(sg_Vector3 a, sg_Vector3 b);

/* Function: sg_Vector3_mul
Multiplies two vectors together using the useful (x1*x2,y1*y2,z1*z2).
*/
sg_Vector3 sg_Vector3_mul(sg_Vector3 a, sg_Vector3 b);

/* Function: sg_Vector3_mul_f
Multiplies all the values in a given vector by a given number.
*/
sg_Vector3 sg_Vector3_mul_f(sg_Vector3 a, float b);

/* Function: sg_Vector3_div
Divides two vectors using (x1/x2,y1/y2,z1/z2).
*/
sg_Vector3 sg_Vector3_div(sg_Vector3 a, sg_Vector3 b);

/* Function: sg_Vector3_div_f
Divides the values in a vector by the given number.
*/
sg_Vector3 sg_Vector3_div_f(sg_Vector3 a, float b);

/* Function: sg_Vector3_len
Uses pythagorean theorem on the given vector, to return the number of units from
the point specified by it to the origin (0,0,0).
*/
float sg_Vector3_len(sg_Vector3 v);

/* Function: sg_Vector3_normalise
Returns a vector whose length is always 1, pointing in the same direction as the
given vector.
*/
sg_Vector3 sg_Vector3_normalise(sg_Vector3 v);

/* Function: sg_Vector3_dot
Returns the dot product of the two vectors.
*/
float sg_Vector3_dot(sg_Vector3 a, sg_Vector3 b);

/* Function: sg_Vector3_cross
Returns the cross product of the two vectors.
*/
sg_Vector3 sg_Vector3_cross(sg_Vector3 a, sg_Vector3 b);

sg_Vector3 sg_Vector3_rotate_q(sg_Vector3 p, sg_Quaternion q);

/* Struct: sg_Vector4
  Four-dimensional vector.
*/

#endif
