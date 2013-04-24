local ffi = require "ffi"

local drawable = require "graphics.drawable"

ffi.cdef [[

struct ilG_drawable3d;

struct ilG_obj_mesh;

struct ilG_drawable3d* ilG_mesh_fromObj(struct ilG_obj_mesh * mesh);

struct ilG_drawable3d* ilG_mesh_fromFile(const char *name);

]]

local mesh = {}

function mesh.create(filename)
    assert(type(filename) == "string");
    return drawable.wrap(ffi.C.ilG_mesh_fromFile(filename));
end

setmetatable(mesh, {__call = function(self, ...) return mesh.create(...) end})

return mesh;

