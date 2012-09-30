#ifndef SG_MATRIX_H
#define SG_MATRIX_H
/* Header: matrix.h
Defines vectors, matrices, and quaternions necessary for sCenegraph.
*/

#include "vector.h"
#include "quaternion.h"

/* Typedef: sg_Matrix
Represents a 4x4 matrix for use in transformation, rotation, and scaling.

Variables:
  data - 16 long array of floats representing the matrix.
*/
typedef struct sg_Matrix {
  float data[16];
} sg_Matrix;

sg_Matrix sg_Matrix_identity;

/* Function: sg_Matrix_mul
Multiplies two matrices together and returns the result.
*/
sg_Matrix sg_Matrix_mul(sg_Matrix a, sg_Matrix b);

/* Function: sg_Vector3_mul_m
Multiplies a vector by a matrix.
*/
sg_Vector4 sg_Vector4_mul_m(sg_Vector4 a, sg_Matrix b);

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

sg_Matrix sg_Matrix_perspective(double fovy, double aspect, double znear, double zfar);

sg_Matrix sg_Matrix_rotate_q(sg_Matrix m, sg_Quaternion q);

#endif

