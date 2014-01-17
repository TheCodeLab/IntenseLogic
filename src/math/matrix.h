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

il_mat il_zero_mat();
il_mat il_mat_new(float d00, float d01, float d02, float d03,
                  float d10, float d11, float d12, float d13,
                  float d20, float d21, float d22, float d23,
                  float d30, float d31, float d32, float d33);
il_mat il_mat_fromarray(float* arr);
il_mat il_mat_mul(const il_mat a, const il_mat b);
il_vec4 il_mat_mulv(const il_mat a, const il_vec4 b);
/** Creates a translation matrix from a vector */
il_mat il_mat_translate(const il_vec4 v);
/** Creates a scaling matrix from a vector */
il_mat il_mat_scale(const il_vec4 v);
/** Creates an identity matrix */
il_mat il_mat_identity();
/** Creates a perspective transform matrix */
il_mat il_mat_perspective(float fovy, float aspect, float znear, float zfar);
/** Creates a rotation matrix */
il_mat il_mat_rotate(const il_quat q);
/** Inverts a matrix */
il_mat il_mat_invert(const il_mat a);
/** Transposes the elements of a matrix over the diagonal */
il_mat il_mat_transpose(const il_mat a);

#endif
