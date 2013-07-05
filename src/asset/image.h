#ifndef ILA_IMAGE_H
#define ILA_IMAGE_H

#include <stdint.h>

#include "asset/node.h"

enum ilA_imgchannels {
    ILA_IMG_R = 1<<0,
    ILA_IMG_G = 1<<1,
    ILA_IMG_B = 1<<2,
    ILA_IMG_A = 1<<3,
    ILA_IMG_RGB = ILA_IMG_R|ILA_IMG_G|ILA_IMG_B,
    ILA_IMG_RGBA = ILA_IMG_RGB | ILA_IMG_A
};

typedef struct ilA_img {
    enum ilA_imgchannels channels;
    unsigned width, height, bpp;
    unsigned char *data;
} ilA_img;

enum ilA_img_interpolation {
    ILA_IMG_NEAREST,
    ILA_IMG_LINEAR
};

ilA_img *ilA_img_load(const void *data, size_t size);
ilA_img *ilA_img_loadasset(const ilA_file *iface, il_base *file);
ilA_img *ilA_img_loadfile(const char *file);
ilA_img *ilA_img_fromdata(const void *data, unsigned w, unsigned h, unsigned depth, enum ilA_imgchannels channels);
void ilA_img_free(ilA_img *self);
ilA_img *ilA_img_resize(const ilA_img *self, enum ilA_img_interpolation up, enum ilA_img_interpolation down, unsigned w, unsigned h, int channels);
ilA_img *ilA_img_swizzle(const ilA_img *self, uint16_t mask);
ilA_img *ilA_img_bgra_to_rgba(const ilA_img *self);

#endif

