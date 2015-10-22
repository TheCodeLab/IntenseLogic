/** @file image.h
 * @brief Image loading and manipulation
 */

#ifndef ILA_IMAGE_H
#define ILA_IMAGE_H

#include <stdint.h>

#include "asset/node.h"
#include "util/types.h"

typedef enum ilA_imgchannel {
    ILA_IMG_R = 1,
    ILA_IMG_RG = 2,
    ILA_IMG_RGB = 3,
    ILA_IMG_RGBA = 4,
} ilA_imgchannel;

typedef enum ilA_imgformat {
    ILA_IMG_U8,
    ILA_IMG_U16,
    ILA_IMG_F32
} ilA_imgformat;

enum ilA_img_interpolation {
    ILA_IMG_NEAREST,    /**< Nearest neighbour interpolation, lowest quality but fastest */
    ILA_IMG_LINEAR,     /**< Linear interpolation, okay for textures */
};

typedef enum ilA_imgerr {
    ILA_IMG_SUCCESS,
    ILA_IMG_UNRECOGNIZED_FORMAT,
    ILA_IMG_NO_SUCH_FILE,
    ILA_IMG_UNKNOWN_INTERPOLATION,
    ILA_IMG_INVALID_MASK,
    ILA_IMG_NULL,
    ILA_IMG_MALLOC_FAILURE,
} ilA_imgerr;

typedef struct ilA_img {
    ilA_imgchannel channels;
    ilA_imgformat format;
    unsigned width, height;
    unsigned char *data;
} ilA_img;

/** Loads an image from memory */
ilA_imgerr IL_WARN_UNUSED ilA_img_load(ilA_img *self, const void *data, size_t size);
/** Loads an image from a file path */
ilA_imgerr IL_WARN_UNUSED ilA_img_loadfile(ilA_img *self, ilA_fs *fs, const char *file);
/** Creates a new image from raw data */
ilA_imgerr IL_WARN_UNUSED ilA_img_fromdata(ilA_img *self, const void *data, unsigned w, unsigned h, ilA_imgformat fmt, ilA_imgchannel chans);
/** Allocates raw data for an image */
ilA_imgerr IL_WARN_UNUSED ilA_img_alloc(ilA_img *self, unsigned w, unsigned h, ilA_imgformat fmt, ilA_imgchannel chans);
/** Allocates a copy of an image */
ilA_imgerr IL_WARN_UNUSED ilA_img_copy(ilA_img *dst, const ilA_img *src);
/** Calculates number of bytes used for a single channel of one pixel */
size_t ilA_img_bytes_per_channel(const ilA_img *self);
/** Calculates number of bytes used for one pixel */
size_t ilA_img_stride(const ilA_img *self);
/** Calculates number of bytes used for one row of the image */
size_t ilA_img_pitch(const ilA_img *self);
/** Calculates number of bytes used to store entire image */
size_t ilA_img_size(const ilA_img *self);
/** Destroys all data associated with an image */
void ilA_img_free(ilA_img self);
/** Resizes an image using the specified interpolation methods */
ilA_imgerr IL_WARN_UNUSED ilA_img_resize(ilA_img *dst, const ilA_img *src, enum ilA_img_interpolation up, enum ilA_img_interpolation down, unsigned w, unsigned h);
/** Swizzles an image */
ilA_imgerr IL_WARN_UNUSED ilA_img_swizzle(ilA_img *dst, const ilA_img *src, uint16_t mask);
/** Converts from bgra to rgba */
ilA_imgerr IL_WARN_UNUSED ilA_img_bgra_to_rgba(ilA_img *dst, const ilA_img *src);
/** Computes a normal map from a height map */
ilA_imgerr IL_WARN_UNUSED ilA_img_height_to_normal(ilA_img *dst, const ilA_img *src);
/** Returns a string for the error code */
const char *ilA_img_strerror(ilA_imgerr err);

#endif
