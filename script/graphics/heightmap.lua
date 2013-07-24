local ffi = require "ffi"

local base = require "common.base"

ffi.cdef [[

struct ilG_drawable3d *ilG_heightmap_new(struct ilG_context *context, unsigned w, unsigned h);
struct ilG_material *ilG_heightmap_shader(struct ilG_context *context);

extern il_type ilG_heightmap_type;

]]

base.wrap "il.graphics.heightmap" {
    struct = "ilG_drawable3d";
    __call = function(self, ...)
        return modules.graphics.ilG_heightmap_new(...)
    end;
    defaultShader = modules.graphics.ilG_heightmap_shader;
}

return modules.graphics.ilG_heightmap_type

