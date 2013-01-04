local ffi = require "ffi"

ffi.cdef [[

struct ilG_texture *ilG_texture_default;

struct ilG_texture* ilG_texture_fromfile(const char *name);
struct ilG_texture* ilG_texture_fromasset(struct ilA_asset* asset);

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

function texture.fromfile(name)
    assert(type(name) == "string", "Expected string");
    return wrap(ffi.C.ilG_texture_fromfile(name));
end

function texture.fromasset(asset)
    assert(ffi.istype("struct ilA_asset*", asset), "Bad argument #1 to fromasset, expected asset");
    return wrap(ffi.C.ilG_texture_fromasset(asset.ptr));
end


return texture;

