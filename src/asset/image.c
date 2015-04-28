#include "image.h"

#include <assert.h>
#include <png.h>

#include "util/log.h"

static int check_png(const void *data, size_t size)
{
    (void)size;
    return !png_sig_cmp(data, 0, 8);
}

struct read_context {
    const unsigned char *data;
    size_t size;
    size_t head;
};

static void png_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    struct read_context *self = (struct read_context*)png_get_io_ptr(png_ptr);
    memcpy(data, self->data + self->head, length);
    self->head += length;
}

static int read_png(ilA_img *self, const void *data, size_t size)
{
    int ctype = 0, bpp, rowbytes;
    unsigned i;
    png_structp png_ptr;
    png_infop info_ptr;
    struct read_context read_context;
    png_bytepp rows;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
        NULL, NULL);
    if (!png_ptr) {
        il_error("Failed to allocate png structure");
        return 0;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        il_error("Failed to allocate png info structure");
        return 0;
    }

    read_context = (struct read_context){data, size, 8};
    png_set_read_fn(png_ptr, &read_context, &png_read_fn);
    png_set_sig_bytes(png_ptr, 8);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_SCALE_16 |
        PNG_TRANSFORM_PACKING, NULL);
    png_get_IHDR(png_ptr, info_ptr, &self->width, &self->height, &bpp, &ctype,
        NULL, NULL, NULL);
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    switch (ctype) {
    case PNG_COLOR_TYPE_RGB_ALPHA:
        self->channels = ILA_IMG_RGBA;
        break;
    case PNG_COLOR_TYPE_RGB:
        self->channels = ILA_IMG_RGB;
        break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
        self->channels = ILA_IMG_RG;
        break;
    case PNG_COLOR_TYPE_GRAY:
        self->channels = ILA_IMG_R;
        break;
    case PNG_COLOR_TYPE_PALETTE:
        il_error("NYI: Palettes");
        break;
    default:
        il_error("Unknown colour type");
    }

    self->data = calloc(rowbytes, self->height);
    rows = png_get_rows(png_ptr, info_ptr);
    for (i = 0; i < self->height; i++) {
        memcpy(self->data + rowbytes*i, rows[i], rowbytes);
    }
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    return 1;
}

ilA_imgerr ilA_img_load(ilA_img *img, const void *data, size_t size)
{
    memset(img, 0, sizeof(ilA_img));
    int res;

    if (check_png(data, size)) { // it is, in fact, a png file
        res = read_png(img, data, size);
        if (res) {
            return ILA_IMG_SUCCESS;
        }
    }

    return ILA_IMG_UNRECOGNIZED_FORMAT;
}

ilA_imgerr ilA_img_loadfile(ilA_img *img, ilA_fs *fs, const char *name)
{
    ilA_map map;
    if (!ilA_mapfile(fs, &map, ILA_READ, name, -1)) {
        ilA_printerror(&map.err);
        return ILA_IMG_NO_SUCH_FILE;
    }
    ilA_imgerr res = ilA_img_load(img, map.data, map.size);
    ilA_unmapfile(&map);
    return res;
}

ilA_imgerr ilA_img_fromdata(ilA_img *img, const void *data, unsigned w, unsigned h, ilA_imgformat fmt, enum ilA_imgchannel chans)
{
    ilA_imgerr res = ilA_img_alloc(img, w, h, fmt, chans);
    if (res) {
        return res;
    }
    size_t size = ilA_img_size(img);
    if (data) {
        memcpy(img->data, data, size);
    } else {
        memset(img->data, 0, size);
    }
    return ILA_IMG_SUCCESS;
}

ilA_imgerr il_important ilA_img_alloc(ilA_img *self, unsigned w, unsigned h, ilA_imgformat fmt, ilA_imgchannel chans)
{
    memset(self, 0, sizeof(ilA_img));
    self->width = w;
    self->height = h;
    self->format = fmt;
    self->channels = chans;
    self->data = malloc(ilA_img_size(self));
    return self->data? ILA_IMG_SUCCESS : ILA_IMG_MALLOC_FAILURE;
}

ilA_imgerr ilA_img_copy(ilA_img *dst, const ilA_img *src)
{
    ilA_imgerr res = ilA_img_alloc(dst, src->width, src->height, src->format, src->channels);
    if (res) {
        return res;
    }
    memcpy(dst->data, src->data, ilA_img_size(src));
    return ILA_IMG_SUCCESS;
}

size_t ilA_img_bytes_per_channel(const ilA_img *self)
{
    switch (self->format) {
    case ILA_IMG_U8: return sizeof(uint8_t);
    case ILA_IMG_U16: return sizeof(uint16_t);
    case ILA_IMG_F32: return sizeof(float);
    }
    assert(!"Invalid format");
    return 0;
}

size_t ilA_img_stride(const ilA_img *self)
{
    return ilA_img_bytes_per_channel(self) * self->channels;
}

size_t ilA_img_pitch(const ilA_img *self)
{
    return ilA_img_stride(self) * self->width;
}

size_t ilA_img_size(const ilA_img *self)
{
    return ilA_img_pitch(self) * self->height;
}

void ilA_img_free(ilA_img self)
{
    free(self.data);
}

static inline void __attribute__((always_inline))
sample_pixel(unsigned char *dst, const unsigned char *src,
             ilA_imgformat dstfmt, ilA_imgchannel dstchans,
             ilA_imgformat srcfmt, ilA_imgchannel srcchans)
{
    assert(dstfmt == srcfmt); // TODO: Format translation
    (void)srcfmt;
    unsigned bytes = 0;
    switch (dstfmt) {
    case ILA_IMG_U8:  bytes = sizeof(uint8_t);  break;
    case ILA_IMG_U16: bytes = sizeof(uint16_t); break;
    case ILA_IMG_F32: bytes = sizeof(float);    break;
    }
    for (unsigned i = 0; i < (dstchans < srcchans? dstchans : srcchans); i++) {
        memcpy(dst, src, bytes);
        dst += bytes;
        src += bytes;
    }
    for (unsigned i = 0; i < srcchans - dstchans; i++) {
        memset(dst, 0, bytes);
        dst += bytes;
    }
}

static inline void __attribute__((always_inline))
nearest_sample(ilA_img *dst, const ilA_img *src, int x, int y)
{
    float xp = (float)x/dst->width, yp = (float)y/dst->height;
    int xs = xp * src->width, ys = yp * src->height;
    unsigned dstpix = y * ilA_img_pitch(dst) + x * ilA_img_stride(dst);
    unsigned srcpix = ys * ilA_img_pitch(src) + xs * ilA_img_stride(src);
    sample_pixel(dst->data + dstpix, src->data + srcpix,
                 dst->format, dst->channels,
                 src->format, src->channels);
}

ilA_imgerr ilA_img_resize(ilA_img *dst, const ilA_img *src, enum ilA_img_interpolation up, enum ilA_img_interpolation down, unsigned w, unsigned h)
{
    enum ilA_img_interpolation interp = w > src->width && h > src->height? up : down;
    unsigned x, y;
    ilA_imgerr res = ilA_img_alloc(dst, w, h, src->format, src->channels);
    if (res) {
        return res;
    }

    switch (interp) {
    case ILA_IMG_NEAREST:
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++) {
                nearest_sample(dst, src, x, y);
            }
        }
        break;
    default:
        return ILA_IMG_UNKNOWN_INTERPOLATION;
    }
    return ILA_IMG_SUCCESS;
}

static void decompose_pixel(ilA_imgformat fmt, ilA_imgchannel chans, const uint8_t *data, uint32_t out[4])
{
    for (unsigned i = 0; i < chans; i++) {
        switch (fmt) {
        case ILA_IMG_U8:
            out[i] = ((const uint8_t*)data)[i];
            break;
        case ILA_IMG_U16:
            out[i] = ((const uint16_t*)data)[i];
            break;
        case ILA_IMG_F32:
            out[i] = ((const uint32_t*)data)[i];
            break;
        }
    }
}

static void compose_pixel(ilA_imgformat fmt, ilA_imgchannel chans, const uint32_t col[4], uint8_t *out)
{
    for (unsigned i = 0; i < chans; i++) {
        switch (fmt) {
        case ILA_IMG_U8:
            ((uint8_t*)out)[i] = col[i];
            break;
        case ILA_IMG_U16:
            ((uint16_t*)out)[i] = col[i];
            break;
        case ILA_IMG_F32:
            ((uint32_t*)out)[i] = col[i];
            break;
        }
    }
}

static void bitmat_mulv(uint16_t mat, const unsigned col[4], unsigned out[4])
{
    int i, j, bit;

    memset(out, 0, sizeof(unsigned char) * 4);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            bit = mat & (1<<(i*4 + j));
            if (bit) {
                out[i] = col[j];
                break;
            }
        }
    }
}

ilA_imgerr ilA_img_swizzle(ilA_img *dst, const ilA_img *src, uint16_t mask)
{
    ilA_imgchannel num_chans = 0;
    for (unsigned i = 0; i < 4; i++) {
        for (unsigned j = 0; j < 4; j++) {
            unsigned bit = mask & (1<<(i*4 + j));
            if (bit) {
                num_chans++;
            }
        }
    }
    if (num_chans < 1 || num_chans > 4) {
        return ILA_IMG_INVALID_MASK;
    }
    ilA_imgerr res = ilA_img_alloc(dst, src->width, src->height, src->format, num_chans);
    if (res) {
        return res;
    }
    unsigned x, y;
    for (y = 0; y < src->height; y++) {
        for (x = 0; x < src->width; x++) {
            unsigned col[4], out[4];
            size_t stride = ilA_img_stride(src);
            size_t pitch = ilA_img_pitch(src);
            decompose_pixel(src->format, src->channels, src->data + y*pitch + x*stride, col);
            bitmat_mulv(mask, col, out);
            compose_pixel(src->format, src->channels, out, dst->data + y*pitch + x*stride);
        }
    }
    return ILA_IMG_SUCCESS;
}

ilA_imgerr ilA_img_bgra_to_rgba(ilA_img *dst, const ilA_img *src)
{
    ilA_imgerr res = ilA_img_alloc(dst, src->width, src->height, src->format, src->channels);
    if (res) {
        return res;
    }
    for (unsigned y = 0; y < src->height; y++) {
        for (unsigned x = 0; x < src->width; x++) {
            size_t pitch = ilA_img_pitch(src), stride = ilA_img_stride(src);
            unsigned char       *dstp = dst->data + y * pitch + x * stride;
            const unsigned char *srcp = src->data + y * pitch + x * stride;
            dstp[0] = srcp[2];
            dstp[1] = srcp[1];
            dstp[2] = srcp[0];
            dstp[3] = srcp[3];
        }
    }
    return ILA_IMG_SUCCESS;
}

const char *ilA_img_strerror(ilA_imgerr err)
{
    static const char *const table[] = {
        "Success",
        "Unrecognized image file format",
        "No such file exists",
        "Interpolation algorithm not supported",
        "Invalid swizzle mask",
        "Null pointer input",
        "Malloc failed"
    };
    if (err >= sizeof(table) / sizeof(const char * const)) {
        return "Invalid error code";
    }
    return table[err];
}
