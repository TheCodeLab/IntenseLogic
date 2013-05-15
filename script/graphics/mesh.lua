--- A wrapper for meshes
-- Inherits `graphics.drawable`.
-- @type mesh
local ffi = require "ffi"
local base = require "common.base"
local drawable = require "graphics.drawable"

ffi.cdef [[

extern il_type ilG_mesh_type;

ilG_drawable3d* ilG_mesh_fromFile(const char *name);

]]

base.wrap "il.graphics.mesh" {
    struct = "ilG_drawable3d";
    --- Creates a mesh from a file
    -- @function fromFile
    -- @tparam string name The filename
    fromFile = modules.graphics.ilG_mesh_fromFile;
    --- Calls `mesh:fromFile`
    __call = function(self, file)
        return modules.graphics.ilG_mesh_fromFile(file)
    end;
}

return modules.graphics.ilG_mesh_type

