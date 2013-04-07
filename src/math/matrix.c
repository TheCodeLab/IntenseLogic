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
    memset(mat, 0, sizeof(float) * 16);
    for(i=0; i<4; i++) {
        for(j=0; j<4; j++) {
            //mat[i*4+j]=0;
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
    memset(m, 0, sizeof(float) * 16);
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

/*static void dividecol(il_mat m, int c, float f)
{
    m[0  + c] /= f;
    m[4  + c] /= f;
    m[8  + c] /= f;
    m[12 + c] /= f;
}*/

static void addrow(il_mat m, int src, int dst, float f)
{
    m[dst*4 + 0] += m[src*4 + 0] * f;
    m[dst*4 + 1] += m[src*4 + 1] * f;
    m[dst*4 + 2] += m[src*4 + 2] * f;
    m[dst*4 + 3] += m[src*4 + 3] * f;
}

static void swaprow(il_mat m, int src, int dst)
{
    float r[4];
    memcpy(r,           m + 4*dst,  sizeof(r));
    memcpy(m + 4*dst,   m + 4*src,  sizeof(r));
    memcpy(m + 4*src,   r,          sizeof(r));
}

static float* getcolumn(const il_mat m, int c, float *res)
{
    res[0] = m[0 + c];
    res[1] = m[4 + c];
    res[2] = m[8 + c];
    res[3] = m[12 + c];
    return res;
}

static il_mat eliminate(const il_mat a, il_mat a2, il_mat res)
{
    if (!res) {
        res = il_mat_copy(a);
    } else {
        memcpy(res, a, sizeof(float)*16);
    }
    int i, j, k;

    for (i = 0; i < 3; i++) {
        int p = -1;
        for (k = i; k < 4; k++) {
            if (res[k*4+i] != 0.f) {
                p = k;
                break;
            }
        }
        if (p == -1) {
            return NULL;
        }
        if (p != i) {
            swaprow(res, p, i);
            if (a2) {
                swaprow(a2, p, i);
            }
        }
        for (j = i+1; j < 4; j++) {
            float m = res[j*4+i] / res[i*5];
            addrow(res, i, j, m);
            if (a2) {
                addrow(a2, i, j, m);
            }
        }
    }
    return res;
}

static float *substitute(const il_mat a, const float *b, float *x)
{
    int i, j;
    // backwards substitution
    if (a[15] == 0.f) {
        return NULL; // not possible
    }
    // x_n = b[n] / a_nn
    x[3] = b[3] / a[15];
    // x_i = (a_i(n+1) - sum(j=i+1, n, a_ij x_j)) / a_ii; for each i = n-1 .. 1
    for (i = 2; i >= 0; i--) {
        float sum = 0.f;
        for (j = i+1; j < 4; j++) {
            sum += a[i*4+j] * x[j];
        }
        x[i] = b[i] - sum;
    }

    return x;

}

il_mat il_mat_invert(const il_mat a, il_mat res)
{
    int allocated = 0;
    if (!res) {
        res = il_mat_new();
        allocated = 1;
    }
#ifdef IL_SSE

#else
    il_mat id = il_mat_identity(NULL);
    int n;
    float col[4];
    il_mat A = eliminate(a, id, NULL);
    
    for (n = 0; n < 4; n++) {
        getcolumn(id, n, col);
        float *x = substitute(A, col, calloc(4, sizeof(float)));
        if (!x) {
            if (allocated) {
                il_mat_free(res);
            }
            res = NULL;
            break;
        }
        res[0  + n] = x[0];
        res[4  + n] = x[1];
        res[8  + n] = x[2];
        res[12 + n] = x[3];
        //memcpy(res, x, sizeof(float) * 4);
        free(x);
    }
    il_mat_free(id);
#endif
    return res;
}

