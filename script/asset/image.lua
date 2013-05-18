local ffi = require "ffi"

require "common.base"
require "asset.file"

ffi.cdef [[

enum ilA_imgchannels {
    ILA_IMG_R = 1<<1,
    ILA_IMG_G = 1<<2,
    ILA_IMG_B = 1<<3,
    ILA_IMG_A = 1<<4,
    ILA_IMG_RGB = ILA_IMG_R|ILA_IMG_G|ILA_IMG_B,
    ILA_IMG_RGBA = ILA_IMG_RGB | ILA_IMG_A
};

typedef struct ilA_img {
    enum ilA_imgchannels channels;
    unsigned width, height, bpp;
    unsigned char *data;
} ilA_img;

ilA_img *ilA_img_load(const void *data, size_t size);
ilA_img *ilA_img_loadasset(ilA_file *iface, il_base *file);
ilA_img *ilA_img_loadfile(const char *file);
void ilA_img_free(ilA_img *self);

]]

local img = {}

function img.load(...)
    local i = modules.asset.ilA_img_load
    ffi.gc(i, modules.ilA_img_free)
    return i
end

function img.loadasset(b) 
    return modules.asset.ilA_img_loadasset(nil, b) 
end

img.loadfile = modules.asset.ilA_img_loadfile

return img

