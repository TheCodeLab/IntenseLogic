/** @file vector.h
 * @brief Vector operations
 */

#ifndef IL_VECTOR_H
#define IL_VECTOR_H

#include "quaternion.h"

/** 16-byte double-precision 2-dimensional vector */
typedef struct il_vec2 {
    float x, y;
} il_vec2;
/** 16-byte float-precision 4-dimensional vector */
typedef struct il_vec4 {
    float x, y, z, w;
} il_vec4;

typedef struct il_vec3 {
    float x, y, z, w;
} il_vec3;

/** Sets the components of an il_vec4 */
il_vec4 il_vec4_new(float x, float y, float z, float w);
/** Sets the components of an il_vec3 */
il_vec3 il_vec3_new(float x, float y, float z);
/** Converts an il_vec4 to a string */
char *il_vec4_print(il_vec4 v, char *buf, unsigned length);
/** Converts an il_vec3 to a string */
char *il_vec3_print(il_vec3 v, char *buf, unsigned length);

/** Sets the components of an il_vec4 from an float array */
il_vec4 il_vec4_fromarr(const float* arr);

il_vec4 il_vec4_add(il_vec4 a, il_vec4 b);
il_vec4 il_vec4_sub(il_vec4 a, il_vec4 b);
il_vec4 il_vec4_mul(il_vec4 a, il_vec4 b);
il_vec4 il_vec4_div(il_vec4 a, il_vec4 b);
/** Computes the dot product of two vectors */
float il_vec4_dot(il_vec4 a, il_vec4 b);
il_vec3 il_vec4_to_vec3(il_vec4 a);
/** Returns the result of pythagorean theorem on the vector */
float il_vec4_len(il_vec4 a);

il_vec3 il_vec3_add(il_vec3 a, il_vec3 b);
il_vec3 il_vec3_sub(il_vec3 a, il_vec3 b);
il_vec3 il_vec3_mul(il_vec3 a, il_vec3 b);
il_vec3 il_vec3_div(il_vec3 a, il_vec3 b);
/** Rotates a vector around the origin using a rotation specified by a quaternion */
il_vec3 il_vec3_rotate(il_vec3 a, il_quat q);
/** Computes the cross product of two vectors */
il_vec3 il_vec3_cross(il_vec3 a, il_vec3 b);
/** Computes the dot product of two vectors */
float il_vec3_dot(il_vec3 a, il_vec3 b);
/** Creates a unit vector of the vector */
il_vec3 il_vec3_normal(il_vec3 a);
il_vec4 il_vec3_to_vec4(il_vec3 a, float w);
/** Returns the result of pythagorean theorem on the vector */
float il_vec3_len(il_vec3 a);

il_vec2 il_vec2_add(il_vec2 a, il_vec2 b);
il_vec2 il_vec2_sub(il_vec2 a, il_vec2 b);
il_vec2 il_vec2_mul(il_vec2 a, il_vec2 b);
il_vec2 il_vec2_div(il_vec2 a, il_vec2 b);

#endif
