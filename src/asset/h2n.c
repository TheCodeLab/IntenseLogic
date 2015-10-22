#include "image.h"

#include "math/vector.h"

static void pixel_height_to_normal(ilA_img *dst, const ilA_img *src, unsigned x, unsigned y)
{
    size_t pitch = ilA_img_pitch(src), stride = ilA_img_stride(src);
    float height[3];
    static const int offs[6] = {
        1, 1,
        0, 1,
        1, 0,
    };
    for (unsigned i = 0; i < 3; i++) {
        unsigned lx = x + offs[2*i], ly = y + offs[2*i + 1];
        uint32_t hvalu;
        float hvalf;
        switch (src->format) {
        case ILA_IMG_U8:
            hvalu = *((uint8_t*)src->data + ly * pitch + lx * stride);
            hvalf = hvalu / 255.f;
            break;
        case ILA_IMG_U16:
            hvalu = *((uint16_t*)src->data + ly * pitch + lx * stride);
            hvalf = hvalu / 65535.f;
            break;
        case ILA_IMG_F32:
            hvalf = *((float*)src->data + ly * pitch + lx * stride);
            break;
        }
        height[i] = hvalf;
    }
    float w = -1.f/src->width, h = -1.f/src->height;
    il_vec3 vx  = il_vec3_new(w, height[1] - height[0], 0);
    il_vec3 vy  = il_vec3_new(0, height[2] - height[0], h);
    il_vec3 res = il_vec3_cross(vy, vx);
    res = il_vec3_normal(res);

    {
        size_t pitch = ilA_img_pitch(dst), stride = ilA_img_stride(dst);
        float *data = (float*)(dst->data + y * pitch + x * stride);
        data[0] = res.x;
        data[1] = res.y;
        data[2] = res.z;
    }
}

ilA_imgerr ilA_img_height_to_normal(ilA_img *dst, const ilA_img *src)
{
    if (!src) {
        return ILA_IMG_NULL;
    }
    ilA_imgerr res = ilA_img_alloc(dst, src->width-1, src->height-1, ILA_IMG_F32, ILA_IMG_RGB);
    if (res) {
        return res;
    }

    for (unsigned y = 0; y < dst->width; y++) {
        for (unsigned x = 0; x < dst->width; x++) {
            pixel_height_to_normal(dst, src, x, y);
        }
    }

    return ILA_IMG_SUCCESS;
}
