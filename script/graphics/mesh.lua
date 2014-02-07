--- A wrapper for meshes
-- Inherits `graphics.drawable`.
-- @type mesh
local ffi = require "ffi"
local base = require "common.base"
local drawable = require "graphics.drawable"

require "asset.mesh"

ffi.cdef [[

extern il_type ilG_mesh_type;

ilG_drawable3d* ilG_mesh_fromfile(const char *name, ilG_context *context);
ilG_drawable3d* ilG_mesh(ilA_mesh* self, ilG_context *context);

]]

base.wrap "il.graphics.mesh" {
    struct = "ilG_drawable3d";
    --- Creates a mesh from a file
    -- @function fromFile
    -- @tparam string name The filename
    fromfile = modules.graphics.ilG_mesh_fromfile;
    --- Creates a mesh from a file path or from an ilA_mesh
    __call = function(self, arg, ctx)
        assert(ctx ~= nil)
        if type(arg) == "string" then
            return modules.graphics.ilG_mesh_fromfile(arg, ctx)
        end
        return modules.graphics.ilG_mesh(arg, ctx)
    end;
}

return modules.graphics.ilG_mesh_type

