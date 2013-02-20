local ffi = require "ffi"

local tunit = require "textureunit"

ffi.cdef [[

struct ilG_texture *ilG_texture_default;

struct ilG_texture* ilG_texture_new();
void ilG_texture_setName(struct ilG_texture* self, const char *name);
void ilG_texture_fromfile(struct ilG_texture* self, unsigned unit, const char *name);
void ilG_texture_fromasset(struct ilG_texture* self, unsigned unit, struct ilA_asset* asset);

]]

local texture = {}

local function wrap(ptr)
    local obj = {}
    obj.ptr = ptr;
    setmetatable(obj, {__index = texture})
    return obj;
end
texture.wrap = wrap

texture.default = wrap(ffi.C.ilG_texture_default);

function texture:setName(name)
    ffi.C.ilG_texture_setName(self.ptr, name)
end

function texture:fromfile(unit, name)
    assert(type(name) == "string", "Expected string");
    return wrap(ffi.C.ilG_texture_fromfile(self.ptr, tunit.toUnit(unit), name));
end

function texture:fromasset(unit, asset)
    assert(ffi.istype("struct ilA_asset*", asset), "Bad argument #1 to fromasset, expected asset");
    return wrap(ffi.C.ilG_texture_fromasset(self.ptr, tunit.toUnit(unit), asset.ptr));
end

function texture.create()
    return wrap(ffi.C.ilG_texture_new())
end

setmetatable(texture, {__call = function(self, ...) return texture.create(...) end})

return texture;

