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

enum ilG_arrayattrib {
    ILG_ARRATTR_POSITION,
    ILG_ARRATTR_TEXCOORD,
    ILG_ARRATTR_NORMAL,
    ILG_ARRATTR_AMBIENT,
    ILG_ARRATTR_DIFFUSE,
    ILG_ARRATTR_SPECULAR,
    ILG_ARRATTR_ISTRANSPARENT,
    ILG_ARRATTR_NUMATTRS,
};

extern il_type ilG_drawable3d_type;

ilG_drawable3d* ilG_box(struct ilG_context *context);
ilG_drawable3d* ilG_icosahedron(struct ilG_context *context);

]]

base.wrap "il.graphics.drawable3d" {
    struct = "ilG_drawable3d";
    setattr = function(self, attr, val)
        attr = ffi.C["ILG_ARRATTR_"..attr:upper()]
        if val then
            self.attrs = bit.bor(tonumber(self.attrs), bit.lshift(1, attr))
        else
            self.attrs = bit.band(tonumber(self.attrs), bit.bnot(bit.lshift(1, attr)))
        end
    end;
    getattr = function(self, attr)
        attr = ffi.C["ILG_ARRATTR_"..attr:upper()]
        return bit.band(self.attrs, bit.lshift(1, attr)) == bit.lshift(1, attr)
    end;
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

