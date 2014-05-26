#include "quaternion.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

il_quat il_quat_new(float x, float y, float z, float w)
{
    il_quat q;
    q.x = x;
    q.y = y;
    q.z = z;
    q.w = w;
    return q;
}

il_quat il_quat_mul(il_quat a, il_quat b)
{
    il_quat n;
    n.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    n.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    n.y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
    n.z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;
    return n;
}

il_quat il_quat_lerp(il_quat a, il_quat b, float t)
{
    il_quat q;
    q.x = a.x * (1-t) + b.x * t;
    q.y = a.y * (1-t) + b.y * t;
    q.z = a.z * (1-t) + b.z * t;
    q.w = a.w * (1-t) + b.w * t;
    return il_quat_normalize(q);
}

il_quat il_quat_fromYPR(float yaw, float pitch, float roll)
{
    il_quat q;
    float c1 = cos(yaw/2);
    float c2 = cos(pitch/2);
    float c3 = cos(roll/2);
    float s1 = sin(yaw/2);
    float s2 = sin(pitch/2);
    float s3 = sin(roll/2);
    q.x = (c1 * c2 * c3) - (s1 * s2 * s3);
    q.y = (s1 * s2 * c3) + (c1 * c2 * s3);
    q.z = (s1 * c2 * c3) + (c1 * s2 * s3);
    q.w = (c1 * s2 * c3) - (s1 * c2 * s3);
    return q;

}

il_quat il_quat_fromAxisAngle(float x, float y, float z, float a)
{
    il_quat q;
    float s = sinf(a/2);
    if (abs(x*x + y*y + z*z - 1) > 0.001) {
        float m = 1/sqrt(x*x+y*y+z*z);
        x *= m;
        y *= m;
        z *= m;
    }
    q.x = s * x;
    q.y = s * y;
    q.z = s * z;
    q.w = cosf(a/2);
    return il_quat_normalize(q);
}

il_quat il_quat_normalize(il_quat a)
{
    il_quat q;
    float len = il_quat_len(a);
    if (len > 0) {
        float ilen = 1/len;
        q.x = a.x * ilen;
        q.y = a.y * ilen;
        q.z = a.z * ilen;
        q.w = a.w * ilen;
    } else {
        q.x = 0;
        q.y = 0;
        q.z = 0;
        q.w = 1;
    }
    return q;
}

il_quat il_quat_invert(il_quat q)
{
    il_quat r;
    r.x = -q.x;
    r.y = -q.y;
    r.z = -q.z;
    r.w = q.w;
    return r;
}

float il_quat_len(il_quat a)
{
    return sqrt(il_quat_dot(a, a));
}

float il_quat_dot(il_quat a, il_quat b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

char *il_quat_print(il_quat q, char *buf, unsigned len)
{
    unsigned flen = snprintf(NULL, 0, "[% .2f % .2f % .2f % .2f]", q.x, q.y, q.z, q.w);
    ++flen;
    if (!buf || flen > len) {
        buf = realloc(buf, flen);
    }
    snprintf(buf, flen, "[% .2f % .2f % .2f % .2f]", q.x, q.y, q.z, q.w);
    return buf;
}

