#ifndef IL_VECTOR_H
#define IL_VECTOR_H

#include "math/quaternion.h"

typedef double *il_vec2;
typedef float *il_vec4;

il_vec4 il_vec4_new();
il_vec2 il_vec2_new();
void il_vec4_free(il_vec4 vec);
void il_vec2_free(il_vec2 vec);
il_vec4 il_vec4_copy(il_vec4 vec);
il_vec2 il_vec2_copy(il_vec2 vec);

il_vec4 il_vec4_set(il_vec4 vec, float x, float y, float z, float w);

il_vec4 il_vec4_add(const il_vec4 a, const il_vec4 b, il_vec4 vec);
il_vec4 il_vec4_sub(const il_vec4 a, const il_vec4 b, il_vec4 vec);
il_vec4 il_vec4_mul(const il_vec4 a, const il_vec4 b, il_vec4 vec);
il_vec4 il_vec4_div(const il_vec4 a, const il_vec4 b, il_vec4 vec);
il_vec4 il_vec4_rotate(const il_vec4 a, const il_quat q, il_vec4 vec);
il_vec4 il_vec4_cross(const il_vec4 a, const il_vec4 b, il_vec4 vec);
float il_vec4_dot(const il_vec4 a, const il_vec4 b);

il_vec2 il_vec2_add(il_vec2 a, il_vec2 b, il_vec2 vec);
il_vec2 il_vec2_sub(il_vec2 a, il_vec2 b, il_vec2 vec);
il_vec2 il_vec2_mul(il_vec2 a, il_vec2 b, il_vec2 vec);
il_vec2 il_vec2_div(il_vec2 a, il_vec2 b, il_vec2 vec);

#endif

