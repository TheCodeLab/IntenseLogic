// Copyright (c) 2012 Code Lab
//
// This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
//
//     2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
//
//     3. This notice may not be removed or altered from any source distribution.
//

#include "vector.h"

#include <math.h>

////////////////////////////////////////////////////////////////////////////////
// Vector2

#define simple_op(name, op) \
sg_Vector2 sg_Vector2_##name(sg_Vector2 a, sg_Vector2 b) { \
  return (sg_Vector2) {a.x op b.x, a.y op b.y}; \
}\
sg_Vector2 sg_Vector2_##name##_f(sg_Vector2 a, float b) { \
  return (sg_Vector2) {a.x op b, a.y op b}; \
}

simple_op(add, +)
simple_op(sub, -)
simple_op(mul, *)
simple_op(div, /)

#undef simple_op

float sg_Vector2_len(sg_Vector2 v) {
  return sqrt(v.x*v.x + v.y*v.y);
}

sg_Vector2 sg_Vector2_normalise(sg_Vector2 v) {
  float l = sg_Vector2_len(v);
  return (sg_Vector2) {v.x/l, v.y/l};
}

float sg_Vector2_dot(sg_Vector2 a, sg_Vector2 b) {
  return (a.x*b.x) + (a.y*b.y);
}

////////////////////////////////////////////////////////////////////////////////
// Vector3

#define simple_op(name, op) \
sg_Vector3 sg_Vector3_##name(sg_Vector3 a, sg_Vector3 b) { \
  return (sg_Vector3) {a.x op b.x, a.y op b.y, a.z op b.z}; \
}\
sg_Vector3 sg_Vector3_##name##_f(sg_Vector3 a, float b) { \
  return (sg_Vector3) {a.x op b, a.y op b, a.z op b}; \
}

simple_op(add, +)
simple_op(sub, -)
simple_op(mul, *)
simple_op(div, /)

#undef simple_op

float sg_Vector3_len(sg_Vector3 v) {
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

sg_Vector3 sg_Vector3_normalise(sg_Vector3 v) {
  float l = sg_Vector3_len(v);
  return (sg_Vector3) {v.x/l, v.y/l, v.z/l};
}

float sg_Vector3_dot(sg_Vector3 a, sg_Vector3 b) {
  return (a.x*b.x) + (a.y*b.y) + (a.z*b.z);
}

// u = (a,b,c) and v = (p,r,q)
// u x v = (br-cq,cp-ar,aq-bp)
sg_Vector3 sg_Vector3_cross(sg_Vector3 a, sg_Vector3 b) {
  return (sg_Vector3) {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}

// pRot = p + 2*cross(q.xyz, q.w*p + cross(q.xyz, p))
sg_Vector3 sg_Vector3_rotate_q(sg_Vector3 p, sg_Quaternion q) {
  return sg_Vector3_add (
    p, 
    sg_Vector3_mul_f (
      sg_Vector3_cross (
        (sg_Vector3){q.x,q.y,q.z}, 
        sg_Vector3_add (
          sg_Vector3_mul_f (
            p, 
            q.w
          ),
          sg_Vector3_cross (
            (sg_Vector3){q.x,q.y,q.z}, 
            p
          )
        )
      ),
      2.0f
    )
  );
}
