local ffi = require "ffi";

ffi.cdef [[

struct ilG_material* ilG_material_default;

]]

local material = {};

local function wrap(ptr)
    local obj = {}
    obj.ptr = ptr;
    setmetatable(obj, {__index=material})
    return obj;
end
material.wrap = wrap;

material.default = wrap(ffi.C.ilG_material_default);

return material;

