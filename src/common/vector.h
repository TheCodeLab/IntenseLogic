#ifndef SG_VECTOR
#define SG_VECTOR

typedef struct il_Vector2 {
  float x, y;
} il_Vector2;

typedef struct il_Vector3 {
  float x, y, z;
} il_Vector3;

typedef struct il_Vector4 {
  float x,y,z,w;
} il_Vector4;

#endif

#ifndef SG_VECTOR_H
#define SG_VECTOR_H

#include "quaternion.h"

/* Vector naming convention

Vector functions have the following prototype:
ret vtype_op[_argtype] (vtype, ... [, argtype]);

*/

/* Typedef: il_Vector2
A type used for representing 2-dimensional vectors.

Variables:
  x, y - The coordinates
*/

il_Vector2 il_Vector2_add(il_Vector2 a, il_Vector2 b);
il_Vector2 il_Vector2_add_f(il_Vector2 a, float b);
il_Vector2 il_Vector2_sub(il_Vector2 a, il_Vector2 b);
il_Vector2 il_Vector2_sub_f(il_Vector2 a, float b);
il_Vector2 il_Vector2_mul(il_Vector2 a, il_Vector2 b);
il_Vector2 il_Vector2_mul_f(il_Vector2 a, float b);
il_Vector2 il_Vector2_div(il_Vector2 a, il_Vector2 b);
il_Vector2 il_Vector2_div_f(il_Vector2 a, float b);

/* Function: il_Vector2_len
Uses pythagorean theorem on the given vector, to return the number of units from
the point specified by it to the origin (0,0).
*/
float il_Vector2_len(il_Vector2 v);

/* Function: il_Vector2_normalise
Returns a vector whose length is always 1, pointing in the same direction as the
given vector.
*/
il_Vector2 il_Vector2_normalise(il_Vector2 v);

/* Function: il_Vector2_dot
Returns the dot product of the two vectors.
*/
float il_Vector2_dot(il_Vector2 a, il_Vector2 b);


/* Typedef: il_Vector3
A type used for representing 3-dimensional vectors.

Variables:
  x, y, z - The coordinates
*/

il_Vector3 il_Vector3_add(il_Vector3 a, il_Vector3 b);
il_Vector3 il_Vector3_add_f(il_Vector3 a, float b);
il_Vector3 il_Vector3_sub(il_Vector3 a, il_Vector3 b);
il_Vector3 il_Vector3_sub_f(il_Vector3 a, float b);
il_Vector3 il_Vector3_mul(il_Vector3 a, il_Vector3 b);
il_Vector3 il_Vector3_mul_f(il_Vector3 a, float b);
il_Vector3 il_Vector3_div(il_Vector3 a, il_Vector3 b);
il_Vector3 il_Vector3_div_f(il_Vector3 a, float b);

/* Function: il_Vector3_len
Uses pythagorean theorem on the given vector, to return the number of units from
the point specified by it to the origin (0,0,0).
*/
float il_Vector3_len(il_Vector3 v);

/* Function: il_Vector3_normalise
Returns a vector whose length is always 1, pointing in the same direction as the
given vector.
*/
il_Vector3 il_Vector3_normalise(il_Vector3 v);

/* Function: il_Vector3_dot
Returns the dot product of the two vectors.
*/
float il_Vector3_dot(il_Vector3 a, il_Vector3 b);

/* Function: il_Vector3_cross
Returns the cross product of the two vectors.
*/
il_Vector3 il_Vector3_cross(il_Vector3 a, il_Vector3 b);

il_Vector3 il_Vector3_rotate_q(il_Vector3 p, il_Quaternion q);

/* Typedef: il_Vector4
A type used for representing 4-dimensional vectors.

Variables:
  x, y, z - The coordinates
*/

il_Vector4 il_Vector4_add(il_Vector4 a, il_Vector4 b);
il_Vector4 il_Vector4_add_f(il_Vector4 a, float b);
il_Vector4 il_Vector4_sub(il_Vector4 a, il_Vector4 b);
il_Vector4 il_Vector4_sub_f(il_Vector4 a, float b);
il_Vector4 il_Vector4_mul(il_Vector4 a, il_Vector4 b);
il_Vector4 il_Vector4_mul_f(il_Vector4 a, float b);
il_Vector4 il_Vector4_div(il_Vector4 a, il_Vector4 b);
il_Vector4 il_Vector4_div_f(il_Vector4 a, float b);

/* Function: il_Vector4_len
Uses pythagorean theorem on the given vector, to return the number of units from
the point specified by it to the origin (0,0,0).
*/
float il_Vector4_len(il_Vector4 v);

/* Function: il_Vector4_normalise
Returns a vector whose length is always 1, pointing in the same direction as the
given vector.
*/
il_Vector4 il_Vector4_normalise(il_Vector4 v);

/* Function: il_Vector4_dot
Returns the dot product of the two vectors.
*/
float il_Vector4_dot(il_Vector4 a, il_Vector4 b);

/* Function: il_Vector4_cross
Returns the cross product of the two vectors.
*/
il_Vector4 il_Vector4_cross(il_Vector4 a, il_Vector4 b);

il_Vector4 il_Vector4_rotate_q(il_Vector4 p, il_Quaternion q);

/* Struct: il_Vector4
  Four-dimensional vector.
*/

#endif
