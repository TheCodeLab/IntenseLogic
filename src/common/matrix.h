#ifndef MATRIX_H
#define MATRIX_H
/* Header: matrix.h
Defines vectors, matrices, and quaternions necessary for sCenegraph.
*/


/* Typedef: sg_Vector2
A type used for representing 2-dimensional vectors.

Variables:
  x, y - The coordinates
*/
typedef struct sg_Vector2 {
  float x, y;
} sg_Vector2;

/* Function: sg_Vector2_add
Adds two vectors together using (x1+x2, y1+y2).
*/
sg_Vector2 sg_Vector2_add(sg_Vector2 a, sg_Vector2 b);

/* Function: sg_Vector2_sub
sg_Vector2_add except subtracting.
*/
sg_Vector2 sg_Vector2_sub(sg_Vector2 a, sg_Vector2 b);

/* Function: sg_Vector2_mul
Multiplies two vectors together using the useful (x1*x2,y1*y2).
*/
sg_Vector2 sg_Vector2_mul(sg_Vector2 a, sg_Vector2 b);

/* Function: sg_Vector2_mul_f
Multiplies all the values in a given vector by a given number.
*/
sg_Vector2 sg_Vector2_mul_f(sg_Vector2 a, float b);

/* Function: sg_Vector2_div
Divides two vectors using (x1/x2,y1/y2).
*/
sg_Vector2 sg_Vector2_div(sg_Vector2 a, sg_Vector2 b);

/* Function: sg_Vector2_div_f
Divides the values in a vector by the given number.
*/
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
typedef struct sg_Vector3 {
  float x, y, z;
} sg_Vector3;

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


/* Typedef: sg_Matrix
Represents a 4x4 matrix for use in transformation, rotation, and scaling.

Variables:
  data - 16 long array of floats representing the matrix.
*/
typedef struct sg_Matrix {
  float data[16];
} sg_Matrix;

/* Function: sg_Matrix_mul
Multiplies two matrices together and returns the result.
*/
sg_Matrix sg_Matrix_mul(sg_Matrix a, sg_Matrix b);

/* Function: sg_Matrix_transform
Transforms a given matrix by the given vector and returns the result.
*/
sg_Matrix sg_Matrix_transform(sg_Matrix m, sg_Vector3 t);

/* Function: sg_Matrix_rotate_v
Rotates the given matrix in a way similar to OpenGL's glRotate.
*/
sg_Matrix sg_Matrix_rotate_v(sg_Matrix m, float a, sg_Vector3 n);

/* Function: sg_Matrix_scale
Scales the given matrix by the given vector and returns it.
*/
sg_Matrix sg_Matrix_scale(sg_Matrix m, sg_Vector3 v); 


/* Typedef: sg_Quaternion
Used for representing rotation without gimbal lock.

Variables:
  x, y, z, w - The 4 values used for representing the quaternion.
*/
typedef struct sg_Quaternion {
  float x, y, z, w;
} sg_Quaternion;

/* Function: sg_Quaternion_new
Returns a pointer to a new quaternion, based on a given lookvector and rotation
angle. 
*/
sg_Quaternion sg_Quaternion_new(sg_Vector3 v, float a);

/* Function: sg_Vector3_rotate
Rotates a given vector using a quaternion.
*/
sg_Vector3 sg_Vector3_rotate(sg_Vector3 p, sg_Quaternion q);

/* Function: sg_Matrix_rotate_q
Rotates a matrix using a quaternion.
*/
sg_Matrix sg_Matrix_rotate_q(sg_Matrix m, sg_Quaternion q);

#endif

