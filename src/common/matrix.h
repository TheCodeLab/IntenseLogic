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

sg_Matrix sg_Matrix_mul(sg_Matrix a, sg_Matrix b);

sg_Vector4 sg_Vector4_mul_m(sg_Vector4 a, sg_Matrix b);

sg_Matrix sg_Matrix_translate(sg_Vector3 t);

sg_Matrix sg_Matrix_rotate_v(float a, sg_Vector3 n);

sg_Matrix sg_Matrix_scale(sg_Vector3 v);

sg_Matrix sg_Matrix_perspective(double fovy, double aspect, double znear, double zfar);

sg_Matrix sg_Matrix_rotate_q(sg_Quaternion q);

int sg_Matrix_invert(sg_Matrix m, sg_Matrix* invOut);

#endif

