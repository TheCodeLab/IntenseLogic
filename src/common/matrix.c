#include <stdlib.h>
#include <math.h>

#include "matrix.h"


inline sg_Vector2 sg_Vector2_add(sg_Vector2 a, sg_Vector2 b) {
  return (sg_Vector2) {a.x + b.x, a.y + b.y};
}

inline sg_Vector2 sg_Vector2_sub(sg_Vector2 a, sg_Vector2 b) {
  return (sg_Vector2) {a.x - b.x, a.y - b.y};
}

inline sg_Vector2 sg_Vector2_mul(sg_Vector2 a, sg_Vector2 b) {
  return (sg_Vector2) {a.x * b.x, a.y * b.y};
}

inline sg_Vector2 sg_Vector2_mul_f(sg_Vector2 a, float b) {
  return (sg_Vector2) {a.x * b, a.y * b};
}

inline sg_Vector2 sg_Vector2_div(sg_Vector2 a, sg_Vector2 b) {
  return (sg_Vector2) {a.x / b.x, a.y / b.y};
}

inline sg_Vector2 sg_Vector2_div_f(sg_Vector2 a, float b) {
  return (sg_Vector2) {a.x / b, a.y / b};
}

inline float sg_Vector2_len(sg_Vector2 v) {
  return sqrt(v.x*v.x + v.y*v.y);
}

inline sg_Vector2 sg_Vector2_normalise(sg_Vector2 v) {
  float l = sg_Vector2_len(v);
  return (sg_Vector2) {v.x/l, v.y/l};
}

inline float sg_Vector2_dot(sg_Vector2 a, sg_Vector2 b) {
  return (a.x*b.x) + (a.y*b.y);
}

// u = (a,b,c) and v = (p,r,q)
// u x v = (br-cq,cp-ar,aq-bp)
/*inline sg_Vector2 sg_Vector2_cross(sg_Vector2 a, sg_Vector2 b) {
  // TODO: 2d version
  return (sg_Vector2) {a.y*b.y - a.z*b.z, a.y*b.x - a.x*b.y, a.x*b.z - a.y*b.x};
}*/


inline sg_Vector3 sg_Vector3_add(sg_Vector3 a, sg_Vector3 b) {
  return (sg_Vector3) {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline sg_Vector3 sg_Vector3_sub(sg_Vector3 a, sg_Vector3 b) {
  return (sg_Vector3) {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline sg_Vector3 sg_Vector3_mul(sg_Vector3 a, sg_Vector3 b) {
  return (sg_Vector3) {a.x * b.x, a.y * b.y, a.z * b.z};
}

inline sg_Vector3 sg_Vector3_mul_f(sg_Vector3 a, float b) {
  return (sg_Vector3) {a.x * b, a.y * b, a.z * b};
}

inline sg_Vector3 sg_Vector3_div(sg_Vector3 a, sg_Vector3 b) {
  return (sg_Vector3) {a.x / b.x, a.y / b.y, a.z / b.z};
}

inline sg_Vector3 sg_Vector3_div_f(sg_Vector3 a, float b) {
  return (sg_Vector3) {a.x / b, a.y / b, a.z / b};
}

inline float sg_Vector3_len(sg_Vector3 v) {
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

inline sg_Vector3 sg_Vector3_normalise(sg_Vector3 v) {
  float l = sg_Vector3_len(v);
  return (sg_Vector3) {v.x/l, v.y/l, v.z/l};
}

inline float sg_Vector3_dot(sg_Vector3 a, sg_Vector3 b) {
  return (a.x*b.x) + (a.y*b.y) + (a.z*b.z);
}

// u = (a,b,c) and v = (p,r,q)
// u x v = (br-cq,cp-ar,aq-bp)
inline sg_Vector3 sg_Vector3_cross(sg_Vector3 a, sg_Vector3 b) {
  return (sg_Vector3) {a.y*b.y - a.z*b.z, a.y*b.x - a.x*b.y, a.x*b.z - a.y*b.x};
}

// pRot = p + 2*cross(q.xyz, q.w*p + cross(q.xyz, p))
inline sg_Vector3 sg_Vector3_rotate(sg_Vector3 p, sg_Quaternion q) {
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


sg_Matrix sg_Matrix_mul(sg_Matrix a, sg_Matrix b) {
  sg_Matrix c;
  
  int row1, row2, col;
  
  for (row1 = 0; row1 < 4; row1++) {
    float p = 1;
    for (col = 0; col < 4; col++) {
      p *= a.data[row1*4 + col];
    }
    
    for (row2 = 0; row2 < 4; row2++) {
      float p2 = 1;
      for (col = 0; col < 4; col++) {
        p2 *= b.data[col*4 + row2];
      }
      
      c.data[row1*4 + row2] = p+p2;
    }
  }
  
  return c;
}

sg_Matrix sg_Matrix_transform(sg_Matrix m, sg_Vector3 t) {
  sg_Matrix n;
  n.data[3] = t.x;
  n.data[7] = t.y;
  n.data[11] = t.z;
  n = sg_Matrix_mul(m, n);
  return n;
}

sg_Matrix sg_Matrix_rotate_v(sg_Matrix m, float a, sg_Vector3 n) {
  sg_Matrix b;
  float c = cosf(a);
  float s = sinf(a);
  b.data[0] = n.x*n.x*(1-c)+c;
  b.data[1] = n.x*n.y*(1-c)-(n.z*s);
  b.data[2] = n.x*n.z*(1-c)+(n.y*s);
  
  b.data[4] = n.y*n.x*(1-c)+(n.z*s);
  b.data[5] = n.y*n.y*(1-c)+c;
  b.data[6] = n.y*n.z*(1-c)-(n.x*s);
  
  b.data[8] = n.x*n.z*(1-c)-(n.y*s);
  b.data[9] = n.y*n.z*(1-c)+(n.x*s);
  b.data[10]= n.z*n.z*(1-c)+c;
  
  b = sg_Matrix_mul(m, b);
  return b;
}

/*
vec3 Q = 2.*q.xyz;
qMat = mat3(
 	1 - Q.y*q.y - Q.z*q.z, Q.x*q.y + Q.z*q.w, Q.x*q.z - Q.y*q.w,
 	Q.x*q.y - Q.z*q.w, 1 - Q.x*q.x - Q.z*q.z, Q.y*q.z + Q.x*q.w,
 	Q.x*q.z + Q.y*q.w, Q.y*q.z - Q.x*q.w, 1 - Q.x*q.x - Q.y*q.y); */
sg_Matrix sg_Matrix_rotate_q(sg_Matrix m, sg_Quaternion q) {
  sg_Vector3 Q = sg_Vector3_mul_f((sg_Vector3){q.x,q.y,q.z},2.0f);
  sg_Matrix n;
  
  n.data[0] = 1 - (Q.y*q.y) - (Q.z*q.z);
  n.data[1] = (Q.x*q.y) + (Q.z*q.w);
  n.data[2] = (Q.x*q.z) - (Q.y*q.w);
  
  n.data[4] = (Q.x*q.y) - (Q.z*q.w);
  n.data[5] = 1 - (Q.x*q.x) - (Q.z*q.z);
  n.data[6] = (Q.y*q.z) + (Q.x*q.w);
  
  n.data[8] = (Q.x*q.z) + (Q.y*q.w);
  n.data[9] = (Q.y*q.z) - (Q.x*q.w);
  n.data[10]= 1 - (Q.x*q.x) - (Q.y*q.y);
  
  n = sg_Matrix_mul(m, n);
  
  return n;
}

sg_Matrix sg_Matrix_scale(sg_Matrix m, sg_Vector3 v) {
  sg_Matrix n;
  n.data[0] = v.x;
  n.data[5] = v.y;
  n.data[10] = v.z;
  
  n = sg_Matrix_mul(m, n);
  
  return n;
}


sg_Quaternion sg_Quaternion_new(sg_Vector3 v, float a) {
  sg_Quaternion q;
  float s = sinf(a/2);
  q.x = s * v.x;
  q.y = s * v.y;
  q.z = s * v.z;
  q.w = cosf(a/2);
  return q;
}

