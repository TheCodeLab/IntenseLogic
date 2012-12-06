#include "matrix.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "quaternion.h"

il_Matrix il_Matrix_identity = {
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }
};

il_Matrix il_Matrix_mul(il_Matrix a, il_Matrix b)
{
    il_Matrix c;

    int i,j,k;
    for(i=0; i<4; i++) {
        for(j=0; j<4; j++) {
            c.data[i*4+j]=0;
            for(k=0; k<4; k++) {
                c.data[i*4+j]+=a.data[i*4+k]*b.data[k*4+j];
            }
        }
    }

    return c;
}

il_Vector4 il_Vector4_mul_m(il_Vector4 vec, il_Matrix b)
{
    float c[4];
    float *a = (float*)&vec;

    int row1, col;

    for (row1 = 0; row1 < 4; row1++) {
        c[row1] = 0;
        for (col = 0; col < 4; col++) {
            c[row1] += a[col] * b.data[row1*4 + col];
        }
    }

    return (il_Vector4) {
        c[0],c[1],c[2],c[3]
    };
}

il_Matrix il_Matrix_translate(il_Vector3 t)
{
    il_Matrix n = il_Matrix_identity;
    n.data[3] = t.x;
    n.data[7] = t.y;
    n.data[11] = t.z;
    return n;
}

il_Matrix il_Matrix_rotate_v(float a, il_Vector3 n)
{
    il_Matrix b = il_Matrix_identity;
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

    return b;
}

/*
   vec3 Q = 2.*q.xyz;
   qMat = mat3(
   1 - Q.y*q.y - Q.z*q.z, Q.x*q.y + Q.z*q.w, Q.x*q.z - Q.y*q.w,
   Q.x*q.y - Q.z*q.w, 1 - Q.x*q.x - Q.z*q.z, Q.y*q.z + Q.x*q.w,
   Q.x*q.z + Q.y*q.w, Q.y*q.z - Q.x*q.w, 1 - Q.x*q.x - Q.y*q.y); */
il_Matrix il_Matrix_rotate_q(il_Quaternion q)
{
    il_Vector3 Q = il_Vector3_mul_f((il_Vector3) {
        q.x,q.y,q.z
    },2.0f);
    il_Matrix n;
    memset(&n, 0, sizeof(il_Matrix));

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

    return n;
}

il_Matrix il_Matrix_scale(il_Vector3 v)
{
    il_Matrix n;
    memset(&n, 0, sizeof(il_Matrix));
    n.data[0] = v.x;
    n.data[5] = v.y;
    n.data[10] = v.z;
    n.data[15] = 1.0;

    return n;
}

// blatantly ripped off from
// http://www.opengl.org/sdk/docs/man/xhtml/gluPerspective.xml
il_Matrix il_Matrix_perspective(double fovy, double aspect, double znear, double zfar)
{
    il_Matrix res;
    memset(&res, 0, sizeof(il_Matrix));

    double f = 1.0/tan(fovy/2);

    res.data[0] = f / aspect;
    res.data[5] = f;
    res.data[10] = (zfar+znear)/(znear-zfar);
    res.data[11] = (2*zfar*znear)/(znear-zfar);
    res.data[14] = -1;

    return res;
}

int il_Matrix_invert(il_Matrix m, il_Matrix* invOut)
{

    double inv[16], det;
    int i;

    inv[0] = m.data[5]  * m.data[10] * m.data[15] -
             m.data[5]  * m.data[11] * m.data[14] -
             m.data[9]  * m.data[6]  * m.data[15] +
             m.data[9]  * m.data[7]  * m.data[14] +
             m.data[13] * m.data[6]  * m.data[11] -
             m.data[13] * m.data[7]  * m.data[10];

    inv[4] = -m.data[4]  * m.data[10] * m.data[15] +
             m.data[4]  * m.data[11] * m.data[14] +
             m.data[8]  * m.data[6]  * m.data[15] -
             m.data[8]  * m.data[7]  * m.data[14] -
             m.data[12] * m.data[6]  * m.data[11] +
             m.data[12] * m.data[7]  * m.data[10];

    inv[8] = m.data[4]  * m.data[9] * m.data[15] -
             m.data[4]  * m.data[11] * m.data[13] -
             m.data[8]  * m.data[5] * m.data[15] +
             m.data[8]  * m.data[7] * m.data[13] +
             m.data[12] * m.data[5] * m.data[11] -
             m.data[12] * m.data[7] * m.data[9];

    inv[12] = -m.data[4]  * m.data[9] * m.data[14] +
              m.data[4]  * m.data[10] * m.data[13] +
              m.data[8]  * m.data[5] * m.data[14] -
              m.data[8]  * m.data[6] * m.data[13] -
              m.data[12] * m.data[5] * m.data[10] +
              m.data[12] * m.data[6] * m.data[9];

    inv[1] = -m.data[1]  * m.data[10] * m.data[15] +
             m.data[1]  * m.data[11] * m.data[14] +
             m.data[9]  * m.data[2] * m.data[15] -
             m.data[9]  * m.data[3] * m.data[14] -
             m.data[13] * m.data[2] * m.data[11] +
             m.data[13] * m.data[3] * m.data[10];

    inv[5] = m.data[0]  * m.data[10] * m.data[15] -
             m.data[0]  * m.data[11] * m.data[14] -
             m.data[8]  * m.data[2] * m.data[15] +
             m.data[8]  * m.data[3] * m.data[14] +
             m.data[12] * m.data[2] * m.data[11] -
             m.data[12] * m.data[3] * m.data[10];

    inv[9] = -m.data[0]  * m.data[9] * m.data[15] +
             m.data[0]  * m.data[11] * m.data[13] +
             m.data[8]  * m.data[1] * m.data[15] -
             m.data[8]  * m.data[3] * m.data[13] -
             m.data[12] * m.data[1] * m.data[11] +
             m.data[12] * m.data[3] * m.data[9];

    inv[13] = m.data[0]  * m.data[9] * m.data[14] -
              m.data[0]  * m.data[10] * m.data[13] -
              m.data[8]  * m.data[1] * m.data[14] +
              m.data[8]  * m.data[2] * m.data[13] +
              m.data[12] * m.data[1] * m.data[10] -
              m.data[12] * m.data[2] * m.data[9];

    inv[2] = m.data[1]  * m.data[6] * m.data[15] -
             m.data[1]  * m.data[7] * m.data[14] -
             m.data[5]  * m.data[2] * m.data[15] +
             m.data[5]  * m.data[3] * m.data[14] +
             m.data[13] * m.data[2] * m.data[7] -
             m.data[13] * m.data[3] * m.data[6];

    inv[6] = -m.data[0]  * m.data[6] * m.data[15] +
             m.data[0]  * m.data[7] * m.data[14] +
             m.data[4]  * m.data[2] * m.data[15] -
             m.data[4]  * m.data[3] * m.data[14] -
             m.data[12] * m.data[2] * m.data[7] +
             m.data[12] * m.data[3] * m.data[6];

    inv[10] = m.data[0]  * m.data[5] * m.data[15] -
              m.data[0]  * m.data[7] * m.data[13] -
              m.data[4]  * m.data[1] * m.data[15] +
              m.data[4]  * m.data[3] * m.data[13] +
              m.data[12] * m.data[1] * m.data[7] -
              m.data[12] * m.data[3] * m.data[5];

    inv[14] = -m.data[0]  * m.data[5] * m.data[14] +
              m.data[0]  * m.data[6] * m.data[13] +
              m.data[4]  * m.data[1] * m.data[14] -
              m.data[4]  * m.data[2] * m.data[13] -
              m.data[12] * m.data[1] * m.data[6] +
              m.data[12] * m.data[2] * m.data[5];

    inv[3] = -m.data[1] * m.data[6] * m.data[11] +
             m.data[1] * m.data[7] * m.data[10] +
             m.data[5] * m.data[2] * m.data[11] -
             m.data[5] * m.data[3] * m.data[10] -
             m.data[9] * m.data[2] * m.data[7] +
             m.data[9] * m.data[3] * m.data[6];

    inv[7] = m.data[0] * m.data[6] * m.data[11] -
             m.data[0] * m.data[7] * m.data[10] -
             m.data[4] * m.data[2] * m.data[11] +
             m.data[4] * m.data[3] * m.data[10] +
             m.data[8] * m.data[2] * m.data[7] -
             m.data[8] * m.data[3] * m.data[6];

    inv[11] = -m.data[0] * m.data[5] * m.data[11] +
              m.data[0] * m.data[7] * m.data[9] +
              m.data[4] * m.data[1] * m.data[11] -
              m.data[4] * m.data[3] * m.data[9] -
              m.data[8] * m.data[1] * m.data[7] +
              m.data[8] * m.data[3] * m.data[5];

    inv[15] = m.data[0] * m.data[5] * m.data[10] -
              m.data[0] * m.data[6] * m.data[9] -
              m.data[4] * m.data[1] * m.data[10] +
              m.data[4] * m.data[2] * m.data[9] +
              m.data[8] * m.data[1] * m.data[6] -
              m.data[8] * m.data[2] * m.data[5];

    det = m.data[0] * inv[0] + m.data[1] * inv[4] + m.data[2] * inv[8] + m.data[3] * inv[12];

    if (det == 0) return -1;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut->data[i] = inv[i] * det;

    return 0;
}
