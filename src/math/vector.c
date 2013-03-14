#include "vector.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#ifdef IL_SSE
#include <xmmintrin.h>
#endif

#include "ilmath.h"

il_vec4 il_vec4_new()
{
    il_vec4 v = il_math_get_policy()->allocate(sizeof(float) * 4);
    return v;
}

il_vec2 il_vec2_new()
{
    il_vec2 v = il_math_get_policy()->allocate(sizeof(double) * 2);
    return v;
}

void il_vec4_free(il_vec4 vec)
{
    il_math_get_policy()->deallocate(vec);
}

void il_vec2_free(il_vec2 vec)
{
    il_math_get_policy()->deallocate(vec);
}

il_vec4 il_vec4_copy(il_vec4 vec)
{
    il_vec4 res = il_vec4_new();
    memcpy(res, vec, sizeof(float) * 4);
    return res;
}

il_vec2 il_vec2_copy(il_vec2 vec)
{
    il_vec2 res = il_vec2_new();
    memcpy(res, vec, sizeof(double) * 2);
    return res;
}

///////////////////////////////////////////////////////////////////////////////
// vec4 operations

il_vec4 il_vec4_set(il_vec4 vec, float x, float y, float z, float w)
{
    if (!vec) {
        vec = il_vec4_new();
    }
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
    vec[3] = w;
    return vec;
}

char *il_vec4_print(const il_vec4 v, char *buf, unsigned length)
{
    unsigned flen = snprintf(NULL, 0, "(% .2f % .2f % .2f % .2f)", v[0], v[1], v[2], v[3]);
    ++flen;
    if (!buf || flen > length) {
        buf = realloc(buf, flen);
    }
    snprintf(buf, flen, "(% .2f % .2f % .2f % .2f)", v[0], v[1], v[2], v[3]);
    return buf;
}

il_vec4 il_vec4_add(const il_vec4 a, const il_vec4 b, il_vec4 vec)
{
    if (!vec) {
       vec = il_vec4_new();
    }
#ifdef IL_SSE
    _mm_store_ps(vec, _mm_add_ps(_mm_load_ps(a), _mm_load_ps(b)));
#else
    vec[0] = a[0] + b[0];
    vec[1] = a[1] + b[1];
    vec[2] = a[2] + b[2];
    vec[3] = a[3] + b[3];
#endif
    return vec;
}

il_vec4 il_vec4_sub(const il_vec4 a, const il_vec4 b, il_vec4 vec)
{
    if (!vec) {
       vec = il_vec4_new();
    }
#ifdef IL_SSE
    _mm_store_ps(vec, _mm_sub_ps(_mm_load_ps(a), _mm_load_ps(b)));
#else
    vec[0] = a[0] - b[0];
    vec[1] = a[1] - b[1];
    vec[2] = a[2] - b[2];
    vec[3] = a[3] - b[3];
#endif
    return vec;
}

il_vec4 il_vec4_mul(const il_vec4 a, const il_vec4 b, il_vec4 vec)
{
    if (!vec) {
       vec = il_vec4_new();
    }
#ifdef IL_SSE
    _mm_store_ps(vec, _mm_mul_ps(_mm_load_ps(a), _mm_load_ps(b)));
#else
    vec[0] = a[0] * b[0];
    vec[1] = a[1] * b[1];
    vec[2] = a[2] * b[2];
    vec[3] = a[3] * b[3];
#endif
    return vec;
}

il_vec4 il_vec4_div(const il_vec4 a, const il_vec4 b, il_vec4 vec)
{
    if (!vec) {
       vec = il_vec4_new();
    }
#ifdef IL_SSE
    _mm_store_ps(vec, _mm_div_ps(_mm_load_ps(a), _mm_load_ps(b)));
#else
    vec[0] = a[0] / b[0];
    vec[1] = a[1] / b[1];
    vec[2] = a[2] / b[2];
    vec[3] = a[3] / b[3];
#endif
    return vec;
}

float il_vec4_dot(const il_vec4 a, const il_vec4 b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

il_vec3 il_vec4_to_vec3(const il_vec4 a, il_vec3 vec)
{
    if (!vec) {
        vec = il_vec3_new();
    }
    vec[0] = a[0] / a[3];
    vec[1] = a[1] / a[3];
    vec[2] = a[2] / a[3];
    vec[3] = 1.0;
    return vec;
}

float il_vec4_len(const il_vec4 a)
{
    return sqrt(il_vec4_dot(a,a));
}

///////////////////////////////////////////////////////////////////////////////
// vec3 operations

il_vec3 il_vec3_rotate(const il_vec3 a, const il_quat q, il_vec3 vec)
{
    if (!vec) {
        vec = il_vec3_new();
    }
    /* From glm/gtc/quaternion.inl
                typename detail::tquat<T>::value_type Two(2);

		detail::tvec3<T> uv, uuv;
		detail::tvec3<T> QuatVector(q.x, q.y, q.z);
		uv = glm::cross(QuatVector, v);
		uuv = glm::cross(QuatVector, uv);
		uv *= (Two * q.w); 
		uuv *= Two; 

		return v + uv + uuv;
    */
    il_vec3 uv, uuv, qv;
    qv = il_vec3_set(NULL, q[0], q[1], q[2]);
    uv = il_vec3_cross(qv, a, NULL);
    uuv = il_vec3_cross(qv, uv, NULL);
    uv[0] *= 2*q[3];
    uv[1] *= 2*q[3];
    uv[2] *= 2*q[3];
    uuv[0] *= 2;
    uuv[1] *= 2;
    uuv[2] *= 2;

    vec = il_vec3_add(a, uv, vec);
    vec = il_vec3_add(vec, uuv, vec);

    il_vec3_free(uv);
    il_vec3_free(uuv);
    il_vec3_free(qv);

    return vec;
}

il_vec3 il_vec3_cross(const il_vec3 a, const il_vec3 b, il_vec3 vec)
{
    if (!vec) {
        vec = il_vec3_new();
    }
    float n[3];
    n[0] = a[1] * b[2] - b[1] * a[2];
    n[1] = a[2] * b[0] - b[2] * a[0];
    n[2] = a[0] * b[1] - b[0] * a[1];
    vec[0] = n[0];
    vec[1] = n[1];
    vec[2] = n[2];
    return vec;
}

float il_vec3_dot(const il_vec3 a, const il_vec3 b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

il_vec3 il_vec3_normal(const il_vec3 a, il_vec3 vec)
{
    if (!vec) {
        vec = il_vec3_new();
    }
    float len = il_vec3_len(a);
    vec[0] = a[0]/len;
    vec[1] = a[1]/len;
    vec[2] = a[2]/len;
    return vec;
}

il_vec4 il_vec3_to_vec4(const il_vec3 a, float w, il_vec4 vec)
{
    if (!vec) {
        vec = il_vec4_new();
    }
    vec[0] = a[0];
    vec[1] = a[1];
    vec[2] = a[2];
    vec[3] = w;
    return vec;
}

float il_vec3_len(const il_vec3 a)
{
    return sqrt(il_vec3_dot(a,a));
}

///////////////////////////////////////////////////////////////////////////////
// vec2 operations

il_vec2 il_vec2_add(il_vec2 a, il_vec2 b, il_vec2 vec)
{
     if (!vec) {
         vec = il_vec2_new();
     }
#ifdef IL_SSE
    _mm_store_pd(vec, _mm_add_pd(_mm_load_pd(a), _mm_load_pd(b)));
#else
    vec[0] = a[0] + b[0];
    vec[1] = a[1] + b[1];
#endif
    return vec;
}

il_vec2 il_vec2_sub(il_vec2 a, il_vec2 b, il_vec2 vec)
{
     if (!vec) {
         vec = il_vec2_new();
     }
#ifdef IL_SSE
    _mm_store_pd(vec, _mm_sub_pd(_mm_load_pd(a), _mm_load_pd(b)));
#else
    vec[0] = a[0] - b[0];
    vec[1] = a[1] - b[1];
#endif
    return vec;
}

il_vec2 il_vec2_mul(il_vec2 a, il_vec2 b, il_vec2 vec)
{
     if (!vec) {
         vec = il_vec2_new();
     }
#ifdef IL_SSE
    _mm_store_pd(vec, _mm_mul_pd(_mm_load_pd(a), _mm_load_pd(b)));
#else
    vec[0] = a[0] * b[0];
    vec[1] = a[1] * b[1];
#endif
    return vec;
}

il_vec2 il_vec2_div(il_vec2 a, il_vec2 b, il_vec2 vec)
{
     if (!vec) {
         vec = il_vec2_new();
     }
#ifdef IL_SSE
    _mm_store_pd(vec, _mm_div_pd(_mm_load_pd(a), _mm_load_pd(b)));
#else
    vec[0] = a[0] / b[0];
    vec[1] = a[1] / b[1];
#endif
    return vec;
}


