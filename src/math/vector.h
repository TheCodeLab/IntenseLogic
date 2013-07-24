/** @file vector.h
 * @brief Vector operations
 */

#ifndef IL_VECTOR_H
#define IL_VECTOR_H

#include "quaternion.h"

/** 16-byte double-precision 2-dimensional vector */
typedef double *il_vec2;
/** 16-byte float-precision 4-dimensional vector */
typedef float *il_vec4;
/** il_vec4 but ignores the fourth component */
typedef float *il_vec3;

/** Returns a new il_vec4 */
il_vec4 il_vec4_new();
/** Returns a new il_vec3 */
#define il_vec3_new il_vec4_new
/** Returns a new il_vec2 */
il_vec2 il_vec2_new();
/** Destroys an il_vec4 */
void il_vec4_free(il_vec4 vec);
/** Destroys an il_vec3 */
#define il_vec3_free il_vec4_free
/** Destroys an il_vec2 */
void il_vec2_free(il_vec2 vec);
/** Copies an il_vec4 */
il_vec4 il_vec4_copy(il_vec4 vec);
/** Copies an il_vec3 */
#define il_vec3_copy il_vec4_copy
/** Copies an il_vec2 */
il_vec2 il_vec2_copy(il_vec2 vec);

/** Sets the components of an il_vec4 */
il_vec4 il_vec4_set(il_vec4 vec, float x, float y, float z, float w);
/** Sets the components of an il_vec3 */
#define il_vec3_set(v, x, y, z) ((il_vec3)il_vec4_set((il_vec4)v, x, y, z, 1.0))
/** Converts an il_vec4 to a string */
char *il_vec4_print(const il_vec4 v, char *buf, unsigned length);
/** Converts an il_vec3 to a string */
char *il_vec3_print(const il_vec3 v, char *buf, unsigned length);

il_vec4 il_vec4_add(const il_vec4 a, const il_vec4 b, il_vec4 vec);
il_vec4 il_vec4_sub(const il_vec4 a, const il_vec4 b, il_vec4 vec);
il_vec4 il_vec4_mul(const il_vec4 a, const il_vec4 b, il_vec4 vec);
il_vec4 il_vec4_div(const il_vec4 a, const il_vec4 b, il_vec4 vec);
/** Computes the dot product of two vectors */
float il_vec4_dot(const il_vec4 a, const il_vec4 b);
il_vec3 il_vec4_to_vec3(const il_vec4 a, il_vec4 vec);
/** Returns the result of pythagorean theorem on the vector */
float il_vec4_len(const il_vec4 a);

#define il_vec3_add(a, b, v) ((il_vec3)il_vec4_add((il_vec4)a, (il_vec4)b, (il_vec4)v))
#define il_vec3_sub(a, b, v) ((il_vec3)il_vec4_sub((il_vec4)a, (il_vec4)b, (il_vec4)v))
#define il_vec3_mul(a, b, v) ((il_vec3)il_vec4_mul((il_vec4)a, (il_vec4)b, (il_vec4)v))
#define il_vec3_div(a, b, v) ((il_vec3)il_vec4_div((il_vec4)a, (il_vec4)b, (il_vec4)v))
/** Rotates a vector around the origin using a rotation specified by a quaternion */
il_vec3 il_vec3_rotate(const il_vec3 a, const il_quat q, il_vec3 vec);
/** Computes the cross product of two vectors */
il_vec3 il_vec3_cross(const il_vec3 a, const il_vec3 b, il_vec3 vec);
/** Computes the dot product of two vectors */
float il_vec3_dot(const il_vec3 a, const il_vec3 b);
/** Creates a unit vector of the vector */
il_vec3 il_vec3_normal(const il_vec3 a, il_vec3 vec);
il_vec4 il_vec3_to_vec4(const il_vec3 a, float w, il_vec4 vec);
/** Returns the result of pythagorean theorem on the vector */
float il_vec3_len(const il_vec3 a);

il_vec2 il_vec2_add(il_vec2 a, il_vec2 b, il_vec2 vec);
il_vec2 il_vec2_sub(il_vec2 a, il_vec2 b, il_vec2 vec);
il_vec2 il_vec2_mul(il_vec2 a, il_vec2 b, il_vec2 vec);
il_vec2 il_vec2_div(il_vec2 a, il_vec2 b, il_vec2 vec);

#endif

