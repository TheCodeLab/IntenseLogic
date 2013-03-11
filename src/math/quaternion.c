#include "quaternion.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "ilmath.h"

il_quat il_quat_new()
{
    il_quat q = il_math_get_policy()->allocate(sizeof(float) * 4);
    q[0] = 0;
    q[1] = 0;
    q[2] = 0;
    q[3] = 1;
    return q;
}

void il_quat_free(il_quat q)
{
    il_math_get_policy()->deallocate(q);
}

il_quat il_quat_copy(il_quat q)
{
    il_quat res = il_quat_new();
    memcpy(res, q, sizeof(float) * 4);
    return res;
}

il_quat il_quat_set(il_quat q, float x, float y, float z, float w)
{
    if (!q) {
        q = il_quat_new();
    }
    q[0] = x;
    q[1] = y;
    q[2] = z;
    q[3] = w;
    return q;
}

il_quat il_quat_mul(const il_quat a, const il_quat b, il_quat q)
{
    if (!q) {
        q = il_quat_new();
    }
    float n[4];
    n[3] = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];
    n[0] = a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1];
    n[1] = a[3] * b[1] + a[1] * b[3] + a[2] * b[0] - a[0] * b[2];
    n[2] = a[3] * b[2] + a[2] * b[3] + a[0] * b[1] - a[1] * b[0];
    q[0] = n[0];
    q[1] = n[1];
    q[2] = n[2];
    q[3] = n[3];
    return q;
}

il_quat il_quat_lerp(const il_quat a, const il_quat b, float t, il_quat q)
{
    if (!q) {
        q = il_quat_new();
    }
    q[0] = a[0] * (1-t) + b[0] * t;
    q[1] = a[1] * (1-t) + b[1] * t;
    q[2] = a[2] * (1-t) + b[2] * t;
    q[3] = a[3] * (1-t) + b[3] * t;
    q = il_quat_normalize(q, q);
    return q;
}

il_quat il_quat_fromYPR(float yaw, float pitch, float roll, il_quat q)
{
    if (!q) {
        q = il_quat_new();
    }
    float c1 = cos(yaw/2);
    float c2 = cos(pitch/2);
    float c3 = cos(roll/2);
    float s1 = sin(yaw/2);
    float s2 = sin(pitch/2);
    float s3 = sin(roll/2);
    q[0] = (c1 * c2 * c3) - (s1 * s2 * s3);
    q[1] = (s1 * s2 * c3) + (c1 * c2 * s3);
    q[2] = (s1 * c2 * c3) + (c1 * s2 * s3);
    q[3] = (c1 * s2 * c3) - (s1 * c2 * s3);
    return q;

}

il_quat il_quat_fromAxisAngle(float x, float y, float z, float a, il_quat q)
{
    if (!q) {
        q = il_quat_new();
    }
    float s = sinf(a/2);
    q[0] = s * x;
    q[1] = s * y;
    q[2] = s * z;
    q[3] = cosf(a/2);
    return il_quat_normalize(q, q);
}

il_quat il_quat_normalize(const il_quat a, il_quat q)
{
    if (!q) {
        q = il_quat_new();
    }
    float len = il_quat_len(a);
    if (len > 0) {
        float ilen = 1/len;
        q[0] = a[0] * ilen;
        q[1] = a[1] * ilen;
        q[2] = a[2] * ilen;
        q[3] = a[3] * ilen;
    } else {
        q[0] = 0;
        q[1] = 0;
        q[2] = 0;
        q[3] = 1;
    }
    return q;
}

float il_quat_len(const il_quat a)
{
    return sqrt(il_quat_dot(a, a));
}

float il_quat_dot(const il_quat a, const il_quat b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

char *il_quat_print(const il_quat q, char *buf, unsigned len)
{
    unsigned flen = snprintf(NULL, 0, "[% .2f % .2f % .2f % .2f]", q[0], q[1], q[2], q[3]);
    ++flen;
    if (!buf || flen > len) {
        buf = realloc(buf, flen);
    }
    snprintf(buf, flen, "[% .2f % .2f % .2f % .2f]", q[0], q[1], q[2], q[3]);
    return buf;
}

