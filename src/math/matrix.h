/** @file matrix.h
 * @brief Matrix operations
 */

#ifndef IL_MATRIX_H
#define IL_MATRIX_H

#include "vector.h"
#include "quaternion.h"

/** 4x4 matrix */
typedef struct il_mat {
  float data[16];
}  __attribute__((aligned(16), packed)) il_mat;

il_mat il_mat_zero();
il_mat il_mat_new(float d00, float d01, float d02, float d03,
                  float d10, float d11, float d12, float d13,
                  float d20, float d21, float d22, float d23,
                  float d30, float d31, float d32, float d33);
il_mat il_mat_fromarray(const float* arr);
il_mat il_mat_mul(il_mat a, il_mat b);
il_vec4 il_mat_mulv(il_mat a, il_vec4 b);
/** Creates a translation matrix from a vector */
il_mat il_mat_translate(il_vec4 v);
/** Creates a scaling matrix from a vector */
il_mat il_mat_scale(il_vec4 v);
/** Creates an identity matrix */
il_mat il_mat_identity();
/** Creates a perspective transform matrix */
il_mat il_mat_perspective(float fovy, float aspect, float znear, float zfar);
/** Creates a rotation matrix */
il_mat il_mat_rotate(il_quat q);
/** Inverts a matrix */
il_mat il_mat_invert(il_mat a);
/** Transposes the elements of a matrix over the diagonal */
il_mat il_mat_transpose(il_mat a);
void il_mat_print(il_mat a);

#endif

