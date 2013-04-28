#ifndef IL_MATRIX_H
#define IL_MATRIX_H

#include "vector.h"
#include "quaternion.h"

typedef float *il_mat;

il_mat il_mat_new();
void il_mat_free(il_mat m);
il_mat il_mat_copy(il_mat m);

il_mat il_mat_set(il_mat m, il_vec4 a, il_vec4 b, il_vec4 c, il_vec4 d);
il_mat il_mat_mul(const il_mat a, const il_mat b, il_mat m);
il_mat il_mat_translate(const il_vec4 v, il_mat m);
il_mat il_mat_scale(const il_vec4 v, il_mat m);
il_mat il_mat_identity(il_mat m);
il_mat il_mat_perspective(il_mat m, float fovy, float aspect, float znear, float zfar);
il_mat il_mat_rotate(const il_quat q, il_mat m);
il_mat il_mat_invert(const il_mat a, il_mat m);
il_mat il_mat_transpose(const il_mat a, il_mat res);

#endif
