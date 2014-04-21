#include "image.h"

#include "math/vector.h"
#include "util/ilassert.h"

ilA_img *ilA_img_height_to_normal(const ilA_img *self)
{
    il_return_null_on_fail(self);
    il_return_null_on_fail(self->channels & ILA_IMG_R);
    ilA_img *img = ilA_img_fromdata(NULL, self->width - 1, self->height - 1, 32, ILA_IMG_RGB);
    unsigned x, y, i;

    img->fp = 1;
    int offs[6] = {
        1, 1,
        0, 1,
        1, 0,
    };
    for (y = 0; y < self->width - 1; y++) {
        for (x = 0; x < self->width - 1; x++) {
            float height[3];
            for (i = 0; i < 3; i++) {
                unsigned lx = x + offs[2*i], ly = y + offs[2*i + 1];
                uint8_t hval = *(uint8_t*)(self->data + ly*self->width*self->bpp/8 + lx*self->bpp/8); // TODO: Support higher than 8-bit
                hval &= (1<<self->bpp) - 1; // we only care about red channel
                height[i] = (float)hval / (1<<self->bpp);
            }
            float w = -1.0/img->width, h = -1.0/img->height;
            //float w = -1, h = -1;
            il_vec3 vx = il_vec3_new(w, height[1] - height[0], 0),
                    vy = il_vec3_new(0, height[2] - height[0], h),
                    res = il_vec3_cross(vy, vx);
            res = il_vec3_normal(res);

            float* data = (float*) (img->data + y*img->width*img->bpp/8 + x*img->bpp/8);
            data[0] = res.x;
            data[1] = res.y;
            data[2] = res.z;
        }
    }

    return img;
}

