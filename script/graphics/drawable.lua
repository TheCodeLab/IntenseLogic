local ffi=require "ffi";

ffi.cdef[[

struct ilG_drawable3d *ilG_box, 
                      *ilG_cylinder, 
                      *ilG_icosahedron, 
                      *ilG_plane;

]]

local drawable = {};

local function wrap(ptr)
    local obj = {};
    obj.ptr = ptr;
    setmetatable(obj, {__index = drawable});
    return obj;
end
drawable.wrap = wrap;

drawable.box = wrap(modules.graphics.ilG_box);
drawable.cylinder = wrap(modules.graphics.ilG_cylinder);
drawable.icosahedron = wrap(modules.graphics.ilG_icosahedron);
drawable.plane = wrap(modules.graphics.ilG_plane);

return drawable;

