#include "vector.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#ifdef IL_SSE
#include <xmmintrin.h>
#endif

#ifndef _ISOC11_SOURCE
void *aligned_alloc(size_t align, size_t size)
{
    char *ptr = malloc(size + align);
    char *aligned = (char*)((size_t)(ptr + align) & (~(align-1)));
    aligned[-1] = aligned - ptr;
    return aligned;
}

void aligned_free(void *ptr)
{
    free((char*)ptr - ((char*)ptr)[-1]);
}
#else
void aligned_free(void* ptr)
{
    free(ptr);
}
#endif

il_vec4 il_vec4_new()
{
    il_vec4 v = aligned_alloc(sizeof(float) * 4, sizeof(float) * 4);
    return v;
}

il_vec2 il_vec2_new()
{
    il_vec2 v = aligned_alloc(sizeof(double) * 2, sizeof(double) * 2);
    return v;
}

void il_vec4_free(il_vec4 vec)
{
    aligned_free(vec);
}

void il_vec2_free(il_vec2 vec)
{
    aligned_free(vec);
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

il_vec4 il_vec4_add(il_vec4 a, il_vec4 b, il_vec4 vec)
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

il_vec4 il_vec4_sub(il_vec4 a, il_vec4 b, il_vec4 vec)
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

il_vec4 il_vec4_mul(il_vec4 a, il_vec4 b, il_vec4 vec)
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

il_vec4 il_vec4_div(il_vec4 a, il_vec4 b, il_vec4 vec)
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

il_vec4 il_vec4_cross(const il_vec4 a, const il_vec4 b, il_vec4 vec)
{
    if (!vec) {
        vec = il_vec4_new();
    }
    vec[0] = a[1] * b[2] - b[1] * a[2];
    vec[1] = a[2] * b[0] - b[2] * a[0];
    vec[2] = a[0] * b[1] - b[0] * a[1];
    return vec;
}

il_vec4 il_vec4_rotate(const il_vec4 a, const il_quat q, il_vec4 vec)
{
    if (!vec) {
        vec = il_vec4_new();
    }
    il_vec4 uv;
    il_vec4 qv = il_vec4_set(NULL, q[0], q[1], q[2], 1.0);
    uv = il_vec4_cross(qv, a, NULL);
    float n = 2 * q[3];
    uv[0] *= n;
    uv[1] *= n;
    uv[2] *= n;
    vec = il_vec4_add(a, uv, vec);
    uv = il_vec4_cross(qv, uv, uv);
    uv[0] *= 2; 
    uv[1] *= 2;
    uv[2] *= 2;
    vec = il_vec4_add(uv, vec, vec);
    il_vec4_free(qv);
    il_vec4_free(uv);

    return vec;
}

float il_vec4_dot(const il_vec4 a, const il_vec4 b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
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


