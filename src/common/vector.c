#include "vector.h"

#include <math.h>

////////////////////////////////////////////////////////////////////////////////
// Vector2

#define simple_op(name, op) \
    il_Vector2 il_Vector2_##name(il_Vector2 a, il_Vector2 b) { \
        return (il_Vector2) {a.x op b.x, a.y op b.y}; \
    }\
il_Vector2 il_Vector2_##name##_f(il_Vector2 a, float b) { \
    return (il_Vector2) {a.x op b, a.y op b}; \
}

simple_op(add, +)
simple_op(sub, -)
simple_op(mul, *)
simple_op(div, /)

#undef simple_op

float il_Vector2_len(il_Vector2 v)
{
    return sqrt(v.x*v.x + v.y*v.y);
}

il_Vector2 il_Vector2_normalise(il_Vector2 v)
{
    float l = il_Vector2_len(v);
    return (il_Vector2) {
        v.x/l, v.y/l
    };
}

float il_Vector2_dot(il_Vector2 a, il_Vector2 b)
{
    return (a.x*b.x) + (a.y*b.y);
}

////////////////////////////////////////////////////////////////////////////////
// Vector3

#define simple_op(name, op) \
    il_Vector3 il_Vector3_##name(il_Vector3 a, il_Vector3 b) { \
        return (il_Vector3) {a.x op b.x, a.y op b.y, a.z op b.z}; \
    }\
il_Vector3 il_Vector3_##name##_f(il_Vector3 a, float b) { \
    return (il_Vector3) {a.x op b, a.y op b, a.z op b}; \
}

simple_op(add, +)
simple_op(sub, -)
simple_op(mul, *)
simple_op(div, /)

#undef simple_op

float il_Vector3_len(il_Vector3 v)
{
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

il_Vector3 il_Vector3_normalise(il_Vector3 v)
{
    float l = il_Vector3_len(v);
    return (il_Vector3) {
        v.x/l, v.y/l, v.z/l
    };
}

float il_Vector3_dot(il_Vector3 a, il_Vector3 b)
{
    return (a.x*b.x) + (a.y*b.y) + (a.z*b.z);
}

// u = (a,b,c) and v = (p,r,q)
// u x v = (br-cq,cp-ar,aq-bp)
il_Vector3 il_Vector3_cross(il_Vector3 a, il_Vector3 b)
{
    return (il_Vector3) {
        a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x
    };
}

/*
   stolen from quaternion.inl in glm source
   template <typename T>
   GLM_FUNC_QUALIFIER detail::tvec3<T> operator*
   (detail::tquat<T> const & q, detail::tvec3<T> const & v) {
   typename detail::tquat<T>::value_type Two(2);

   detail::tvec3<T> uv, uuv;
   detail::tvec3<T> QuatVector(q.x, q.y, q.z);
   uv = glm::cross(QuatVector, v);
   uuv = glm::cross(QuatVector, uv);
   uv *= (Two * q.w);
   uuv *= Two;

   return v + uv + uuv;
   }

// in non-alien:
vec3 rotate(quat q, vec3 v) {
uv = cross(q.xyz, v)
uuv = cross(q.xyz, uv)
uv = uv * (2 * q.w)
uuv = uuv * 2
return v + uv + uuv;
}
*/
il_Vector3 il_Vector3_rotate_q(il_Vector3 v, il_Quaternion q)
{

    il_Vector3 uv, uuv, q_vec;
    q_vec = (il_Vector3) {
        q.x, q.y, q.z
    };
    uv = il_Vector3_cross(q_vec, v);
    uuv = il_Vector3_cross(q_vec, uv);
    uv = il_Vector3_mul_f(uv, 2 * q.w);
    uuv = il_Vector3_mul_f(uuv, 2);

    return il_Vector3_add(
               v,
               il_Vector3_add(
                   uv,
                   uuv
               ));
}

////////////////////////////////////////////////////////////////////////////////
// Vector4

#define simple_op(name, op) \
    il_Vector4 il_Vector4_##name(il_Vector4 a, il_Vector4 b) { \
        return (il_Vector4) {a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w}; \
    }\
il_Vector4 il_Vector4_##name##_f(il_Vector4 a, float b) { \
    return (il_Vector4) {a.x op b, a.y op b, a.z op b, a.w op b}; \
}

simple_op(add, +)
simple_op(sub, -)
simple_op(mul, *)
simple_op(div, /)

#undef simple_op

float il_Vector4_len(il_Vector4 v)
{
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

il_Vector4 il_Vector4_normalise(il_Vector4 v)
{
    float l = il_Vector4_len(v);
    return (il_Vector4) {
        v.x/l, v.y/l, v.z/l, v.w/l
    };
}

float il_Vector4_dot(il_Vector4 a, il_Vector4 b)
{
    return (a.x*b.x) + (a.y*b.y) + (a.z*b.z);
}

// u = (a,b,c) and v = (p,r,q)
// u x v = (br-cq,cp-ar,aq-bp)
il_Vector4 il_Vector4_cross(il_Vector4 a, il_Vector4 b)
{
    return (il_Vector4) {
        a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x, 1
    };
}

il_Vector4 il_Vector4_rotate_q(il_Vector4 v, il_Quaternion q)
{

    il_Vector4 uv, uuv, q_vec;
    q_vec = (il_Vector4) {
        q.x, q.y, q.z, 1
    };
    uv = il_Vector4_cross(q_vec, v);
    uuv = il_Vector4_cross(q_vec, uv);
    uv = il_Vector4_mul_f(uv, 2 * q.w);
    uuv = il_Vector4_mul_f(uuv, 2);

    return il_Vector4_add(
               v,
               il_Vector4_add(
                   uv,
                   uuv
               ));
}

