#ifndef SG_MATRIX_H
#define SG_MATRIX_H
/* Header: matrix.h
Defines vectors, matrices, and quaternions necessary for sCenegraph.
*/

#include "vector.h"
#include "quaternion.h"

/* Typedef: il_Matrix
Represents a 4x4 matrix for use in transformation, rotation, and scaling.

Variables:
  data - 16 long array of floats representing the matrix.
*/
typedef struct il_Matrix {
  float data[16];
} il_Matrix;

il_Matrix il_Matrix_identity;

il_Matrix il_Matrix_mul(il_Matrix a, il_Matrix b);

il_Vector4 il_Vector4_mul_m(il_Vector4 a, il_Matrix b);

il_Matrix il_Matrix_translate(il_Vector3 t);

il_Vector3 il_Matrix_getTranslation(il_Matrix m);

il_Matrix il_Matrix_rotate_v(float a, il_Vector3 n);

il_Matrix il_Matrix_scale(il_Vector3 v);

il_Vector3 il_Matrix_getScale(il_Matrix m);

il_Matrix il_Matrix_perspective(double fovy, double aspect, double znear, double zfar);

il_Matrix il_Matrix_rotate_q(il_Quaternion q);

int il_Matrix_invert(il_Matrix m, il_Matrix* invOut);

il_Matrix il_Matrix_transpose(il_Matrix m);

#endif

