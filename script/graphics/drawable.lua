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

ilG_drawable3d* ilG_box(struct ilG_context *context);
ilG_drawable3d* ilG_icosahedron(struct ilG_context *context);

]]

base.wrap "il.graphics.drawable3d" {
    struct = "ilG_drawable3d";
    --- A simple box mesh
    -- @tparam context context The context to create it in
    -- @treturn drawable3d mesh The mesh
    box         = modules.graphics.ilG_box;
    --- An icosahedron
    -- @tparam context context The context to create it in
    -- @treturn drawable3d mesh The mesh\
    icosahedron = modules.graphics.ilG_icosahedron;
}

return modules.graphics.ilG_drawable3d_type;

