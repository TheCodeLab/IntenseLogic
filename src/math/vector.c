#include "vector.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#ifdef IL_SSE
#include <xmmintrin.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// vec4 operations

il_vec4 il_vec4_new(float x, float y, float z, float w)
{
    il_vec4 vec;
    vec.x = x;
    vec.y = y;
    vec.z = z;
    vec.w = w;
    return vec;
}
il_vec4 il_vec4_fromarr(float* arr)
{
    il_vec4 vec;
    vec.x = arr[0];
    vec.y = arr[1];
    vec.z = arr[2];
    vec.w = arr[3];
    return vec;    
}

char *il_vec4_print(const il_vec4 v, char *buf, unsigned length)
{
    unsigned flen = snprintf(NULL, 0, "(% .2f % .2f % .2f % .2f)", v.x, v.y, v.z, v.w);
    ++flen;
    if (!buf || flen > length) {
        buf = realloc(buf, flen);
    }
    snprintf(buf, flen, "(% .2f % .2f % .2f % .2f)", v.x, v.y, v.z, v.w);
    return buf;
}

il_vec4 il_vec4_add(const il_vec4 a, const il_vec4 b)
{
    il_vec4 vec;
#ifdef IL_SSE
    _mm_store_ps(&vec, _mm_add_ps(_mm_load_ps(&a), _mm_load_ps(&b)));
#else
    vec.x = a.x + b.x;
    vec.y = a.y + b.y;
    vec.z = a.z + b.z;
    vec.w = a.w + b.w;
#endif
    return vec;
}

il_vec4 il_vec4_sub(const il_vec4 a, const il_vec4 b)
{
    il_vec4 vec;
#ifdef IL_SSE
    _mm_store_ps(&vec, _mm_sub_ps(_mm_load_ps(&a), _mm_load_ps(&b)));
#else
    vec.x = a.x - b.x;
    vec.y = a.y - b.y;
    vec.z = a.z - b.z;
    vec.w = a.w - b.w;
#endif
    return vec;
}

il_vec4 il_vec4_mul(const il_vec4 a, const il_vec4 b)
{
    il_vec4 vec;
#ifdef IL_SSE
    _mm_store_ps(&vec, _mm_mul_ps(_mm_load_ps(&a), _mm_load_ps(&b)));
#else
    vec.x = a.x * b.x;
    vec.y = a.y * b.y;
    vec.z = a.z * b.z;
    vec.w = a.w * b.w;
#endif
    return vec;
}

il_vec4 il_vec4_div(const il_vec4 a, const il_vec4 b)
{
    il_vec4 vec;
#ifdef IL_SSE
    _mm_store_ps(&vec, _mm_div_ps(_mm_load_ps(&a), _mm_load_ps(&b)));
#else
    vec.x = a.x / b.x;
    vec.y = a.y / b.y;
    vec.z = a.z / b.z;
    vec.w = a.w / b.w;
#endif
    return vec;
}

float il_vec4_dot(const il_vec4 a, const il_vec4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

il_vec3 il_vec4_to_vec3(const il_vec4 a)
{
    il_vec3 vec;
    vec.x = a.x / a.w;
    vec.y = a.y / a.w;
    vec.z = a.z / a.w;
    vec.w = 1.0;
    return vec;
}

float il_vec4_len(const il_vec4 a)
{
    return sqrt(il_vec4_dot(a,a));
}

///////////////////////////////////////////////////////////////////////////////
// vec3 operations

il_vec3 il_vec3_rotate(const il_vec3 a, const il_quat q)
{
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
    il_vec3 uv, uuv, qv, vec;
    qv = il_vec3_new(q.x, q.y, q.z);
    uv = il_vec3_cross(qv, a);
    uuv = il_vec3_cross(qv, uv);
    uv.x *= 2*q.w;
    uv.y *= 2*q.w;
    uv.z *= 2*q.w;
    uuv.x *= 2;
    uuv.y *= 2;
    uuv.z *= 2;

    vec = il_vec3_add(a, uv);
    vec = il_vec3_add(vec, uuv);

    return vec;
}

il_vec3 il_vec3_cross(const il_vec3 a, const il_vec3 b)
{
    il_vec3 vec;
    vec.x = a.y * b.z - b.y * a.z;
    vec.y = a.z * b.x - b.z * a.x;
    vec.z = a.x * b.y - b.x * a.y;
    return vec;
}

float il_vec3_dot(const il_vec3 a, const il_vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

il_vec3 il_vec3_normal(const il_vec3 a)
{
    il_vec3 vec;
    float len = il_vec3_len(a);
    vec.x = a.x/len;
    vec.y = a.y/len;
    vec.z = a.z/len;
    return vec;
}

il_vec4 il_vec3_to_vec4(const il_vec3 a, float w)
{
    il_vec4 vec;
    vec.x = a.x;
    vec.y = a.y;
    vec.z = a.z;
    vec.w = w;
    return vec;
}

float il_vec3_len(const il_vec3 a)
{
    return sqrt(il_vec3_dot(a,a));
}

///////////////////////////////////////////////////////////////////////////////
// vec2 operations

il_vec2 il_vec2_add(il_vec2 a, il_vec2 b)
{
    il_vec2 vec;
#ifdef IL_SSE
    _mm_store_pd(&vec, _mm_add_pd(_mm_load_pd(&a), _mm_load_pd(&b)));
#else
    vec.x = a.x + b.x;
    vec.y = a.y + b.y;
#endif
    return vec;
}

il_vec2 il_vec2_sub(il_vec2 a, il_vec2 b)
{
    il_vec2 vec;
#ifdef IL_SSE
    _mm_store_pd(&vec, _mm_sub_pd(_mm_load_pd(&a), _mm_load_pd(&b)));
#else
    vec.x = a.x - b.x;
    vec.y = a.y - b.y;
#endif
    return vec;
}

il_vec2 il_vec2_mul(il_vec2 a, il_vec2 b)
{
    il_vec2 vec;
#ifdef IL_SSE
    _mm_store_pd(&vec, _mm_mul_pd(_mm_load_pd(&a), _mm_load_pd(&b)));
#else
    vec.x = a.x * b.x;
    vec.y = a.y * b.y;
#endif
    return vec;
}

il_vec2 il_vec2_div(il_vec2 a, il_vec2 b)
{
    il_vec2 vec;
#ifdef IL_SSE
    _mm_store_pd(&vec, _mm_div_pd(_mm_load_pd(&a), _mm_load_pd(&b)));
#else
    vec.x = a.x / b.x;
    vec.y = a.y / b.y;
#endif
    return vec;
}


