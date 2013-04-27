local ffi = require "ffi"

local base = require "common.base"
local tunit = require "graphics.textureunit"

ffi.cdef [[

typedef struct ilG_textureunit {
    int used;
    /*GLuint*/ unsigned int tex;
    /*GLenum*/ unsigned int mode;
} ilG_textureunit;

typedef struct ilG_texture {
    il_base base;
    unsigned int id;
    char *name;
    ilG_textureunit units[162]; // ILG_TUNIT_NUMUNITS
    struct ilG_context *context;
    struct ilG_material *last_mtl;
} ilG_texture;


extern il_type ilG_texture_type;

void ilG_texture_setContext(ilG_texture* self, struct ilG_context *context);
void ilG_texture_setName(ilG_texture* self, const char *name);
void ilG_texture_fromfile(ilG_texture* self, unsigned unit, const char *name);
void ilG_texture_fromasset(ilG_texture* self, unsigned unit, struct ilA_asset* asset);
unsigned int /*GLuint*/ ilG_texture_getRaw(ilG_texture *self, unsigned unit);

]]

base.wrap "il.graphics.texture" {
    struct = "ilG_texture";

    setContext = ffi.C.ilG_texture_setContext;

    setName = ffi.C.ilG_texture_setName;

    fromfile = function(self, unit, name)
        return ffi.C.ilG_texture_fromfile(self, tunit.toUnit(unit), name)
    end;

    fromasset = function(self, unit, asset)
        return ffi.C.ilG_texture_fromasset(self, tunit.toUnit(unit), asset.ptr)
    end;

    getRaw = function(self, unit)
        return ffi.C.ilG_texture_getRaw(self, tunit.toUnit(unit))
    end;
}

return ffi.C.ilG_texture_type

