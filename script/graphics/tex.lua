local ffi = require 'ffi'
local image = require 'asset.image'

ffi.cdef [[

typedef struct ilG_tex {
    unsigned unit;
    _Bool complete;
    unsigned target;
    unsigned object;
    void (*build)(struct ilG_tex *self, struct ilG_context *context);
    void *data;
} ilG_tex;

void ilG_tex_loadfile(ilG_tex *self, const char *file);
void ilG_tex_loadasset(ilG_tex *self, const struct ilA_file *iface, void *file);
void ilG_tex_loadcube(ilG_tex *self, struct ilA_img *faces[6]);
void ilG_tex_loadimage(ilG_tex *self, struct ilA_img *img);

void ilG_tex_bind(ilG_tex *self);
void ilG_tex_build(ilG_tex *self, struct ilG_context *context);

]]

local tex = {}

function tex.create(arg)
    if type(arg) == 'string' then
        return tex.file(arg)
    elseif type(arg) == 'table' then
        return tex.cube(arg)
    elseif ffi.istype('ilA_img', arg) then
        return tex.image(arg)
    else
        error("Can't handle type "..type(arg))
    end
end

function tex.file(f)
    local t = ffi.new("ilG_tex")
    modules.graphics.ilG_tex_loadfile(t, f)
    return t
end

function tex.cube(faces)
    local t = ffi.new("ilG_tex")
    local nfaces = {}
    for i = 1, 6 do
        nfaces[i] = ffi.gc(faces[i]:copy(), nil)
    end
    local i = ffi.new("ilA_img*[6]", nfaces)
    modules.graphics.ilG_tex_loadcube(t, i)
    return t
end

function tex.image(i)
    local t = ffi.new("ilG_tex")
    modules.graphics.ilG_tex_loadimage(t, ffi.gc(i:copy(), nil))
    return t
end

setmetatable(tex, {__call = function(self, ...) return tex.create(...) end})
return tex

