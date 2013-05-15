--- Wrapper for drawable3d
-- See `common.base` for details
-- @type drawable
local ffi=require "ffi"
local base = require "common.base"

ffi.cdef[[

struct ilG_context;

typedef struct ilG_drawable3d ilG_drawable3d;

struct ilG_drawable3d {
    il_base base;
    unsigned long long attrs;
    unsigned int id;
    struct ilG_context *context;
    const char *name;
};

extern il_type ilG_drawable3d_type;
il_type *ilG_drawable3d_type_get();

ilG_drawable3d *ilG_box, 
               *ilG_cylinder, 
               *ilG_icosahedron, 
               *ilG_plane;

]]

base.wrap "il.graphics.drawable3d" {
    struct = "ilG_drawable3d";
    --- A simple box mesh
    box         = modules.graphics.ilG_box;
    --- A cylinder mesh (not implemented)
    cylinder    = modules.graphics.ilG_cylinder;
    --- An icosahedron
    icosahedron = modules.graphics.ilG_icosahedron;
    --- A plane with a vertical-facing normal (not implemented)
    plane       = modules.graphics.ilG_plane;
}

return modules.graphics.ilG_drawable3d_type;

