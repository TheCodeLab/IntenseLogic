#include "matrix.h"

#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <math.h>

// SSE code copied from GLM which is under the MIT license: http://glm.g-truc.net/copying.txt

#include "ilmath.h"

il_mat il_mat_new()
{
    il_mat m = il_math_get_policy()->allocate(sizeof(float) * 16);
    return m;
}

void il_mat_free(il_mat m)
{
    il_math_get_policy()->deallocate(m);     
}

il_mat il_mat_copy(il_mat m)
{
    il_mat res = il_mat_new();
    memcpy(res, m, sizeof(float) * 16);
    return res;
}

il_mat il_mat_set(il_mat m, il_vec4 a, il_vec4 b, il_vec4 c, il_vec4 d)
{
    if (!m) {
        m = il_mat_new();
    }
    memcpy(m + 0,  a, sizeof(float) * 4);
    memcpy(m + 4,  b, sizeof(float) * 4);
    memcpy(m + 8,  c, sizeof(float) * 4);
    memcpy(m + 12, d, sizeof(float) * 4);
    return m;
}

il_mat il_mat_mul(const il_mat a, const il_mat b, il_mat res)
{
    if (!res) {
        res = il_mat_new();
    }
#ifdef IL_SSE
    __mm128 r1[4], r2[4];
    int i;
    for (i = 0; i < 4; i++) {
        r1[i] = _mm_load_ps(a + i*4);
        r2[i] = _mm_load_ps(b + i*4);
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

        _mm_store_ps(res + i*4, a2);
    }
#else
    int i,j,k;
    float mat[16];
    for(i=0; i<4; i++) {
        for(j=0; j<4; j++) {
            mat[i*4+j]=0;
            for(k=0; k<4; k++) {
                mat[i*4+j]+=a[i*4+k]*b[k*4+j];
            }
        }
    }
    memcpy(res, mat, sizeof(float) * 16);
#endif
    return res;
}

il_mat il_mat_translate(const il_vec4 v, il_mat m)
{
    if (!m) {
        m = il_mat_new();
    }
    float data[] = {
        1, 0, 0, v[0],
        0, 1, 0, v[1],
        0, 0, 1, v[2],
        0, 0, 0, v[3],
    };
    memcpy(m, data, sizeof(data));
    return m;
}

il_mat il_mat_scale(const il_vec4 v, il_mat m)
{
    if (!m) {
        m = il_mat_new();
    }
    float data[] = {
        v[0], 0,    0,    0,
        0,    v[1], 0,    0,
        0,    0,    v[2], 0,
        0,    0,    0,    v[3],
    };
    memcpy(m, data, sizeof(data));
    return m;
}

il_mat il_mat_identity(il_mat m)
{
    if (!m) {
        m = il_mat_new();
    }
    float data[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    memcpy(m, data, sizeof(data));
    return m;
}

il_mat il_mat_perspective(il_mat m, float fovy, float aspect, float znear, float zfar)
{
    // http://www.opengl.org/sdk/docs/man/xhtml/gluPerspective.xml
    if (!m) {
        m = il_mat_new();
    }
    memset(m, 0, sizeof(float)*16);
    float f = 1.0/tan(fovy/2);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (zfar+znear)/(znear-zfar);
    m[11] = (2*zfar*znear)/(znear-zfar);
    m[14] = -1;
    return m;
}

il_mat il_mat_rotate(const il_quat q, il_mat m)
{
    if (!m) {
        m = il_mat_new();
    }
    m[0] =  1 - 2 * q[1] * q[1] - 2 * q[2] * q[2];
    m[1] =  2 * q[0] * q[1] + 2 * q[3] * q[2];
    m[2] =  2 * q[0] * q[2] - 2 * q[3] * q[1];
    m[4] =  2 * q[0] * q[1] - 2 * q[3] * q[2];
    m[5] =  1 - 2 * q[0] * q[0] - 2 * q[2] * q[2];
    m[6] =  2 * q[1] * q[2] + 2 * q[3] * q[0];
    m[8] =  2 * q[0] * q[2] + 2 * q[3] * q[1];
    m[9] =  2 * q[1] * q[2] - 2 * q[3] * q[0];
    m[10] = 1 - 2 * q[0] * q[0] - 2 * q[1] * q[1];
    m[15] = 1.0;
    return m;
}

il_mat il_mat_invert(const il_mat a, il_mat res)
{
    if (!res) {
        res = il_mat_new();
    }
#ifdef IL_SSE

#else
    float Coef00 = a[2*4 + 2] * a[3*4 + 3] - a[2*4 + 3] * a[3*4 + 2];
    float Coef02 = a[2*4 + 1] * a[3*4 + 3] - a[2*4 + 3] * a[3*4 + 1];
    float Coef03 = a[2*4 + 1] * a[3*4 + 2] - a[2*4 + 2] * a[3*4 + 1];
    float Coef04 = a[1*4 + 2] * a[3*4 + 3] - a[1*4 + 3] * a[3*4 + 2];
    float Coef06 = a[1*4 + 1] * a[3*4 + 3] - a[1*4 + 3] * a[3*4 + 1];
    float Coef07 = a[1*4 + 1] * a[3*4 + 2] - a[1*4 + 2] * a[3*4 + 1];
    float Coef08 = a[1*4 + 2] * a[2*4 + 3] - a[1*4 + 3] * a[2*4 + 2];
    float Coef10 = a[1*4 + 1] * a[2*4 + 3] - a[1*4 + 3] * a[2*4 + 1];
    float Coef11 = a[1*4 + 1] * a[2*4 + 2] - a[1*4 + 2] * a[2*4 + 1];
    float Coef12 = a[0*4 + 2] * a[3*4 + 3] - a[0*4 + 3] * a[3*4 + 2];
    float Coef14 = a[0*4 + 1] * a[3*4 + 3] - a[0*4 + 3] * a[3*4 + 1];
    float Coef15 = a[0*4 + 1] * a[3*4 + 2] - a[0*4 + 2] * a[3*4 + 1];
    float Coef16 = a[0*4 + 2] * a[2*4 + 3] - a[0*4 + 3] * a[2*4 + 2];
    float Coef18 = a[0*4 + 1] * a[2*4 + 3] - a[0*4 + 3] * a[2*4 + 1];
    float Coef19 = a[0*4 + 1] * a[2*4 + 2] - a[0*4 + 2] * a[2*4 + 1];
    float Coef20 = a[0*4 + 2] * a[1*4 + 3] - a[0*4 + 3] * a[1*4 + 2];
    float Coef22 = a[0*4 + 1] * a[1*4 + 3] - a[0*4 + 3] * a[1*4 + 1];
    float Coef23 = a[0*4 + 1] * a[1*4 + 2] - a[0*4 + 2] * a[1*4 + 1];

    il_vec4 SignA = il_vec4_set(NULL, +1, -1, +1, -1);
    il_vec4 SignB = il_vec4_set(NULL, -1, +1, -1, +1);

    il_vec4 Fac0 = il_vec4_set(NULL, Coef00, Coef00, Coef02, Coef03);
    il_vec4 Fac1 = il_vec4_set(NULL, Coef04, Coef04, Coef06, Coef07);
    il_vec4 Fac2 = il_vec4_set(NULL, Coef08, Coef08, Coef10, Coef11);
    il_vec4 Fac3 = il_vec4_set(NULL, Coef12, Coef12, Coef14, Coef15);
    il_vec4 Fac4 = il_vec4_set(NULL, Coef16, Coef16, Coef18, Coef19);
    il_vec4 Fac5 = il_vec4_set(NULL, Coef20, Coef20, Coef22, Coef23);

    il_vec4 Vec0 = il_vec4_set(NULL, a[0*4 + 1], a[0*4 + 0], a[0*4 + 0], a[0*4 + 0]);
    il_vec4 Vec1 = il_vec4_set(NULL, a[1*4 + 1], a[1*4 + 0], a[1*4 + 0], a[1*4 + 0]);
    il_vec4 Vec2 = il_vec4_set(NULL, a[2*4 + 1], a[2*4 + 0], a[2*4 + 0], a[2*4 + 0]);
    il_vec4 Vec3 = il_vec4_set(NULL, a[3*4 + 1], a[3*4 + 0], a[3*4 + 0], a[3*4 + 0]);

    il_vec4 v1f0 = il_vec4_mul(Vec1, Fac0, NULL);
    il_vec4 v0f0 = il_vec4_mul(Vec0, Fac0, NULL);
    il_vec4 v0f1 = il_vec4_mul(Vec0, Fac1, NULL);
    il_vec4 v0f2 = il_vec4_mul(Vec0, Fac2, NULL);

    il_vec4 v2f1 = il_vec4_mul(Vec2, Fac1, NULL);
    il_vec4 v2f3 = il_vec4_mul(Vec2, Fac3, NULL);
    il_vec4 v1f3 = il_vec4_mul(Vec1, Fac3, NULL);
    il_vec4 v1f4 = il_vec4_mul(Vec1, Fac4, NULL);

    il_vec4 v3f2 = il_vec4_mul(Vec3, Fac2, NULL);
    il_vec4 v3f4 = il_vec4_mul(Vec3, Fac4, NULL);
    il_vec4 v3f5 = il_vec4_mul(Vec3, Fac5, NULL);
    il_vec4 v2f5 = il_vec4_mul(Vec2, Fac5, NULL);

    il_vec4_free(Fac0);
    il_vec4_free(Fac1);
    il_vec4_free(Fac2);
    il_vec4_free(Fac3);
    il_vec4_free(Fac4);

    Vec0 = il_vec4_sub(v1f0, v2f1, Vec0);
    Vec1 = il_vec4_sub(v0f0, v2f3, Vec1);
    Vec2 = il_vec4_sub(v0f1, v1f3, Vec2);
    Vec3 = il_vec4_sub(v0f2, v1f4, Vec3);

    Vec0 = il_vec4_add(Vec0, v3f2, Vec0);
    Vec1 = il_vec4_add(Vec1, v3f4, Vec1);
    Vec2 = il_vec4_add(Vec2, v3f5, Vec2);
    Vec3 = il_vec4_add(Vec3, v2f5, Vec3);

    Vec0 = il_vec4_mul(SignA, Vec0, Vec0);
    Vec1 = il_vec4_mul(SignB, Vec1, Vec1);
    Vec2 = il_vec4_mul(SignA, Vec2, Vec2);
    Vec3 = il_vec4_mul(SignB, Vec3, Vec3);

    il_vec4_free(SignA);
    il_vec4_free(SignB);
    il_vec4_free(v1f0);
    il_vec4_free(v0f0);
    il_vec4_free(v0f1);
    il_vec4_free(v0f2);
    il_vec4_free(v2f1);
    il_vec4_free(v2f3);
    il_vec4_free(v1f3);
    il_vec4_free(v1f4);
    il_vec4_free(v3f2);
    il_vec4_free(v3f4);
    il_vec4_free(v3f5);
    il_vec4_free(v2f5);

    res = il_mat_set(res, Vec0, Vec1, Vec2, Vec3);

    il_vec4_free(Vec0);
    il_vec4_free(Vec1);
    il_vec4_free(Vec2);
    il_vec4_free(Vec3);

    il_vec4 Row0 = il_vec4_set(NULL, res[0], res[1], res[2], res[3]);
    il_vec4 a_col0 = il_vec4_set(NULL, a[0], a[4], a[8], a[12]);

    float Determinant = il_vec4_dot(a_col0, Row0);

    il_vec4_free(Row0);
    il_vec4_free(a_col0);

    int i;
    for (i = 0; i < 16; i++) {
        res[i] /= Determinant;
    }
#endif
    return res;
}

