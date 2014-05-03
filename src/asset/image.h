/** @file image.h
 * @brief Image loading and manipulation
 */

#ifndef ILA_IMAGE_H
#define ILA_IMAGE_H

#include <stdint.h>

#include "asset/node.h"

enum ilA_imgchannels {
    ILA_IMG_R = 1<<0,                               /**< Red */
    ILA_IMG_G = 1<<1,                               /**< Green */
    ILA_IMG_B = 1<<2,                               /**< Blue */
    ILA_IMG_A = 1<<3,                               /**< Alpha */
    ILA_IMG_RG = ILA_IMG_R | ILA_IMG_G,             /**< RG */
    ILA_IMG_RGB = ILA_IMG_RG | ILA_IMG_B,           /**< RGB */
    ILA_IMG_RGBA = ILA_IMG_RGB | ILA_IMG_A          /**< RGBA */
};

typedef struct ilA_img {
    enum ilA_imgchannels channels;
    unsigned width, height, depth, bpp, fp;
    unsigned char *data;
} ilA_img;

enum ilA_img_interpolation {
    ILA_IMG_NEAREST,    /**< Nearest neighbour interpolation, lowest quality but fastest */
    ILA_IMG_LINEAR,     /**< Linear interpolation, okay for textures */
};

/** Loads an image from memory */
ilA_img *ilA_img_load(const void *data, size_t size);
/** Loads an image from a file */
ilA_img *ilA_img_loadasset(const ilA_file *iface, il_base *file);
/** Loads an image from a file path */
ilA_img *ilA_img_loadfile(const char *file);
/** Creates a new image from raw data */
ilA_img *ilA_img_fromdata(const void *data, unsigned w, unsigned h, unsigned depth, enum ilA_imgchannels channels);
/** Creates a new image without copying */
ilA_img *ilA_img_frombuf(void *data, unsigned w, unsigned h, unsigned depth, enum ilA_imgchannels channels);
/** Allocates a copy of an image */
ilA_img *ilA_img_copy(const ilA_img *old);
/** Destroys all data associated with an image */
void ilA_img_free(ilA_img *self);
/** Resizes an image using the specified interpolation methods */
ilA_img *ilA_img_resize(const ilA_img *self, enum ilA_img_interpolation up, enum ilA_img_interpolation down, unsigned w, unsigned h, int channels);
/** Swizzles an image */
ilA_img *ilA_img_swizzle(const ilA_img *self, uint16_t mask);
/** Converts from bgra to rgba */
ilA_img *ilA_img_bgra_to_rgba(const ilA_img *self);
/** Computes a normal map from a height map */
ilA_img *ilA_img_height_to_normal(const ilA_img *self);

#endif

