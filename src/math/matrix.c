#include "matrix.h"

#include <stdlib.h>
#include <string.h>
#ifdef IL_SSE
# include <xmmintrin.h>
#endif
#include <math.h>
#include <assert.h>

// SSE code copied from GLM which is under the MIT license: http://glm.g-truc.net/copying.txt

il_mat il_mat_zero() {
  return il_mat_new(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}
il_mat il_mat_new(float d00, float d01, float d02, float d03,
                  float d10, float d11, float d12, float d13,
                  float d20, float d21, float d22, float d23,
                  float d30, float d31, float d32, float d33) {
    il_mat m;

    m.data[ 0] = d00;
    m.data[ 1] = d01;
    m.data[ 2] = d02;
    m.data[ 3] = d03;

    m.data[ 4] = d10;
    m.data[ 5] = d11;
    m.data[ 6] = d12;
    m.data[ 7] = d13;

    m.data[ 8] = d20;
    m.data[ 9] = d21;
    m.data[10] = d22;
    m.data[11] = d23;

    m.data[12] = d30;
    m.data[13] = d31;
    m.data[14] = d32;
    m.data[15] = d33;

    return m;
}
il_mat il_mat_fromarray(float* arr)
{
  il_mat m;
  memcpy(m.data, arr, sizeof(float)*16);
  return m;
}

il_mat il_mat_mul(const il_mat a, const il_mat b)
{
    il_mat res = il_mat_zero();
#ifdef IL_SSE
    __mm128 r1[4], r2[4];
    int i;
    for (i = 0; i < 4; i++) {
        r1[i] = _mm_load_ps(a.data[i*4]);
        r2[i] = _mm_load_ps(b.data[i*4]);
    }
    for (i = 0; i < 4; i++) {
        __m128 e0 = _mm_shuffle_ps(r2[i], r2[i], _MM_SHUFFLE(0, 0, 0, 0));
        __m128 e1 = _mm_shuffle_ps(r2[i], r2[i], _MM_SHUFFLE(1, 1, 1, 1));
        __m128 e2 = _mm_shuffle_ps(r2[i], r2[i], _MM_SHUFFLE(2, 2, 2, 2));
        __m128 e3 = _mm_shuffle_ps(r2[i], r2[i], _MM_SHUFFLE(3, 3, 3, 3));

        __m128 m0 = _mm_mul_ps(r1[0], e0);
        __m128 m1 = _mm_mul_ps(r1[1], e1);
        __m128 m2 = _mm_mul_ps(r1[2], e2);
        __m128 m3 = _mm_mul_ps(r1[3], e3);

        __m128 a0 = _mm_add_ps(m0, m1);
        __m128 a1 = _mm_add_ps(m2, m3);
        __m128 a2 = _mm_add_ps(a0, a1);

        _mm_store_ps(res.data[i*4], a2);
    }
#else
    int i,j,k;
    for(i=0; i<4; i++) {
        for(j=0; j<4; j++) {
            for(k=0; k<4; k++) {
                res.data[i*4+j]+=a.data[i*4+k]*b.data[k*4+j];
            }
        }
    }
#endif
    return res;
}

// TODO Figure out a better way to do this function.
il_vec4 il_mat_mulv(const il_mat a, const il_vec4 b)
{
    float vec[4];
    memset(vec, 0, sizeof(vec));
    int i;
    for (i = 0; i < 4; i++) {
        vec[i] += a.data[i*4 + 0] * b.x;
        vec[i] += a.data[i*4 + 1] * b.y;
        vec[i] += a.data[i*4 + 2] * b.z;
        vec[i] += a.data[i*4 + 3] * b.w;
    }
    return il_vec4_fromarr(vec);
}

il_mat il_mat_translate(const il_vec4 v)
{
    return il_mat_new (
        1, 0, 0, v.x,
        0, 1, 0, v.y,
        0, 0, 1, v.z,
        0, 0, 0, v.w
    );
}

il_mat il_mat_scale(const il_vec4 v)
{
    return il_mat_new (
        v.x, 0,   0,   0,
        0,   v.y, 0,   0,
        0,   0,   v.z, 0,
        0,   0,   0,   v.w
    );
}

il_mat il_mat_identity()
{
    return il_mat_new (
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}

il_mat il_mat_perspective(float fovy, float aspect, float znear, float zfar)
{
    // http://www.opengl.org/sdk/docs/man/xhtml/gluPerspective.xml
    float f = 1.0/tan(fovy/2);
    float clip0 = (zfar+znear)/(znear-zfar);
    float clip1 = (2*zfar*znear)/(znear-zfar);
    return il_mat_new (
        f / aspect, 0, 0    , 0,
        0         , f, 0    , 0,
        0         , 0, clip0, clip1,
        0         , 0, -1   , 0
    );
}

il_mat il_mat_rotate(const il_quat q)
{
    return il_mat_new (
        1 - 2 * q.y * q.y - 2 * q.z * q.z, 2 * q.x * q.y + 2 * q.w * q.z    , 2 * q.x * q.z - 2 * q.w * q.y    , 0,
        2 * q.x * q.y - 2 * q.w * q.z    , 1 - 2 * q.x * q.x - 2 * q.z * q.z, 2 * q.y * q.z + 2 * q.w * q.x    , 0,
        2 * q.x * q.z + 2 * q.w * q.y    , 2 * q.y * q.z - 2 * q.w * q.x    , 1 - 2 * q.x * q.x - 2 * q.y * q.y, 0,
        0                                , 0                                , 0                                , 1.0
    );
}

static void addrow(il_mat *m, int src, int dst, float f)
{
    m->data[dst*4 + 0] += m->data[src*4 + 0] * f;
    m->data[dst*4 + 1] += m->data[src*4 + 1] * f;
    m->data[dst*4 + 2] += m->data[src*4 + 2] * f;
    m->data[dst*4 + 3] += m->data[src*4 + 3] * f;
}

static void swaprow(il_mat *m, int src, int dst)
{
    float r[4];
    memcpy(r,                 m->data + 4*dst,  sizeof(r));
    memcpy(m->data + 4*dst,   m->data + 4*src,  sizeof(r));
    memcpy(m->data + 4*src,   r,                sizeof(r));
}

static void mulrow(il_mat *m, int col, float f)
{
    m->data[col*4 + 0] *= f;
    m->data[col*4 + 1] *= f;
    m->data[col*4 + 2] *= f;
    m->data[col*4 + 3] *= f;
}

il_mat il_mat_invert(il_mat a)
{
    il_mat res = il_mat_identity();

    // for every column ..
    for (int col = 0; col < 4; ++col) {
        float diagonalfield = a.data[col*4+col];
        // if we have a weird matrix with zero on the diagonal ..
        float eps = 0.0001;
        if (fabs(diagonalfield) < eps) {
            // SIIIGH.
            // Okay, let's go hunting for a row with a better diagonal to use
            // keep in mind, row swapping is also a linear operation!
            for (int row = 0; row < 4; ++row) if (row != col) if (fabs(a.data[row*4+col]) >= eps) {
                swaprow(&a, row, col);
                swaprow(&res,row, col);
            }
            diagonalfield = a.data[col*4+col]; // recompute
            // if it still fails, we probably have a denatured matrix and are proper fucked.
            assert(fabs(diagonalfield) >= eps);
        }
        // for every row ..
        for (int row = 0; row < 4; ++row) {
            // if it's not on the diagonal ..
            if (row != col) {
                float field = a.data[row*4+col];
                // bring this field to 0 by subtracting the row that's on the diagonal for this column
                addrow(&a,  /* src */ col, /* dst */ row, /* factor */ -field/diagonalfield);
                // do the same to identity
                addrow(&res,/* src */ col, /* dst */ row, /* factor */ -field/diagonalfield);
            }
        }
        // and finally, bring the diagonal to 1
        mulrow(&a,  col, 1/diagonalfield);
        mulrow(&res,col, 1/diagonalfield);
    }
    // Success! Because a is now id, res is now a^-1. Math is fun!
    return res;
}

il_mat il_mat_transpose(const il_mat a)
{
    il_mat res = il_mat_identity();
    int x, y;
    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            res.data[y*4 + x] = a.data[x*4 + y];
        }
    }
    return res;
}

