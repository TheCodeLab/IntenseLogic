#ifndef IL_VECTOR_H
#define IL_VECTOR_H

#include "quaternion.h"

typedef double *il_vec2;
typedef float *il_vec4;
typedef float *il_vec3;

il_vec4 il_vec4_new();
#define il_vec3_new il_vec4_new
il_vec2 il_vec2_new();
void il_vec4_free(il_vec4 vec);
#define il_vec3_free il_vec4_free
void il_vec2_free(il_vec2 vec);
il_vec4 il_vec4_copy(il_vec4 vec);
#define il_vec3_copy il_vec4_copy
il_vec2 il_vec2_copy(il_vec2 vec);

il_vec4 il_vec4_set(il_vec4 vec, float x, float y, float z, float w);
#define il_vec3_set(v, x, y, z) ((il_vec3)il_vec4_set((il_vec4)v, x, y, z, 1.0))
char *il_vec4_print(const il_vec4 v, char *buf, unsigned length);
char *il_vec3_print(const il_vec3 v, char *buf, unsigned length);

il_vec4 il_vec4_add(const il_vec4 a, const il_vec4 b, il_vec4 vec);
il_vec4 il_vec4_sub(const il_vec4 a, const il_vec4 b, il_vec4 vec);
il_vec4 il_vec4_mul(const il_vec4 a, const il_vec4 b, il_vec4 vec);
il_vec4 il_vec4_div(const il_vec4 a, const il_vec4 b, il_vec4 vec);
float il_vec4_dot(const il_vec4 a, const il_vec4 b);
il_vec3 il_vec4_to_vec3(const il_vec4 a, il_vec4 vec);
#define il_vec4_len(a) il_vec4_dot(a,a)

// component wise, we can just ignore that it does the w operation
#define il_vec3_add(a, b, v) ((il_vec3)il_vec4_add((il_vec4)a, (il_vec4)b, (il_vec4)v))
#define il_vec3_sub(a, b, v) ((il_vec3)il_vec4_sub((il_vec4)a, (il_vec4)b, (il_vec4)v))
#define il_vec3_mul(a, b, v) ((il_vec3)il_vec4_mul((il_vec4)a, (il_vec4)b, (il_vec4)v))
#define il_vec3_div(a, b, v) ((il_vec3)il_vec4_div((il_vec4)a, (il_vec4)b, (il_vec4)v))
il_vec3 il_vec3_rotate(const il_vec3 a, const il_quat q, il_vec3 vec);
il_vec3 il_vec3_cross(const il_vec3 a, const il_vec3 b, il_vec3 vec);
float il_vec3_dot(const il_vec3 a, const il_vec3 b);
il_vec3 il_vec3_normal(const il_vec3 a, il_vec3 vec);
il_vec4 il_vec3_to_vec4(const il_vec3 a, float w, il_vec4 vec);
#define il_vec3_len(a) il_vec3_dot(a,a)

il_vec2 il_vec2_add(il_vec2 a, il_vec2 b, il_vec2 vec);
il_vec2 il_vec2_sub(il_vec2 a, il_vec2 b, il_vec2 vec);
il_vec2 il_vec2_mul(il_vec2 a, il_vec2 b, il_vec2 vec);
il_vec2 il_vec2_div(il_vec2 a, il_vec2 b, il_vec2 vec);

#endif

