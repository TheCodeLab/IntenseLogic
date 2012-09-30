#include "matrix.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "quaternion.h"

sg_Matrix sg_Matrix_identity = {
  {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  }
};

sg_Matrix sg_Matrix_mul(sg_Matrix a, sg_Matrix b) {
  sg_Matrix c;
  
  int i,j,k;
  for(i=0;i<4;i++){
    for(j=0;j<4;j++){
      c.data[i*4+j]=0;
      for(k=0;k<4;k++){
        c.data[i*4+j]+=a.data[i*4+k]*b.data[k*4+j];
      }
    }
  }
  
  return c;
}

sg_Vector4 sg_Vector4_mul_m(sg_Vector4 vec, sg_Matrix b) {
  float c[4];
  float *a = (float*)&vec;
  
  int row1, col;
  
  for (row1 = 0; row1 < 4; row1++) {
    c[row1] = 0;
    for (col = 0; col < 4; col++) {
      c[row1] += a[col] * b.data[row1*4 + col];
    }
  }
  
  return (sg_Vector4){c[0],c[1],c[2],c[3]};
}

sg_Matrix sg_Matrix_transform(sg_Matrix m, sg_Vector3 t) {
  sg_Matrix n = sg_Matrix_identity;
  n.data[3] = t.x;
  n.data[7] = t.y;
  n.data[11] = t.z;
  return sg_Matrix_mul(m, n);
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
  memset(&n, 0, sizeof(sg_Matrix));
  
  n.data[0] = 1 - (Q.y*q.y) - (Q.z*q.z);
  n.data[1] = (Q.x*q.y) + (Q.z*q.w);
  n.data[2] = (Q.x*q.z) - (Q.y*q.w);
  
  n.data[4] = (Q.x*q.y) - (Q.z*q.w);
  n.data[5] = 1 - (Q.x*q.x) - (Q.z*q.z);
  n.data[6] = (Q.y*q.z) + (Q.x*q.w);
  
  n.data[8] = (Q.x*q.z) + (Q.y*q.w);
  n.data[9] = (Q.y*q.z) - (Q.x*q.w);
  n.data[10]= 1 - (Q.x*q.x) - (Q.y*q.y);
  
  n.data[15] = 1;

  return sg_Matrix_mul(m, n);
}

sg_Matrix sg_Matrix_scale(sg_Matrix m, sg_Vector3 v) {
  sg_Matrix n;
  n.data[0] = v.x;
  n.data[5] = v.y;
  n.data[10] = v.z;
  
  n = sg_Matrix_mul(m, n);
  
  return n;
}

// blatantly ripped off from
// http://www.opengl.org/sdk/docs/man/xhtml/gluPerspective.xml
sg_Matrix sg_Matrix_perspective(double fovy, double aspect, double znear, double zfar) {
  sg_Matrix res;
  memset(&res, 0, sizeof(sg_Matrix));
  
  double f = 1.0/tan(fovy/2);
  
  res.data[0] = f / aspect;
  res.data[5] = f;
  res.data[10] = (zfar+znear)/(znear-zfar);
  res.data[11] = (2*zfar*znear)/(znear-zfar);
  res.data[14] = -1;
  
  return res;
}
