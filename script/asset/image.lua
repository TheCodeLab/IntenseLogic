local ffi = require "ffi"

require "common.base"
require "asset.file"

ffi.cdef [[

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
ilA_img *ilA_img_loadasset(ilA_file *iface, il_base *file);
ilA_img *ilA_img_loadfile(const char *file);
void ilA_img_free(ilA_img *self);
ilA_img *ilA_img_resize(const ilA_img *self, enum ilA_img_interpolation up, enum ilA_img_interpolation down, unsigned w, unsigned h, int channels);
ilA_img *ilA_img_swizzle(const ilA_img *self, uint16_t mask);

]]

local img = {}

function img.load(...)
    local i = modules.asset.ilA_img_load
    ffi.gc(i, modules.asset.ilA_img_free)
    return i
end

function img.loadasset(b) 
    return modules.asset.ilA_img_loadasset(nil, b) 
end

img.loadfile = modules.asset.ilA_img_loadfile

function img:resize(w, h, channels, up, down)
    if not up then          up          = "nearest" end
    if not down then        down        = up        end
    if not channels then    channels    = "RGBA"    end
    assert(type(w) == "number" and type(h) == "number")
    local interps = {["nearest"]=0, ["linear"]=1}
    local chans = {R=1, G=2, B=4, A=8}
    up = interps[up] or 0
    down = interps[up] or 0
    local bits = 0
    for i=1, #channels do
        local chan = chans[channels:sub(i,i)]
        if not chan then error("Unknown channel configuration: "..channels) end
        bits = bit.bor(bits, chan)
    end
    return ffi.gc(modules.asset.ilA_img_resize(self, up, down, w, h, bits), modules.asset.ilA_img_free)
end

function img:swizzle(str)
    local mat = 0
    local chans = {r=1, g=2, b=3, a=4}
    --[[
    -- j=* R G B A
    -- i=1 
    -- i=2 
    -- i=3 
    -- i=4 
    --]]
    for i = 1, #str do
        local j = chans[str:sub(i,i)]
        mat = bit.bor(mat, bit.bshift(1, i*4 + j))
    end
    return modules.asset.ilA_img_swizzle(self, mat)
end

ffi.metatype("ilA_img", {__index=img})

return img

