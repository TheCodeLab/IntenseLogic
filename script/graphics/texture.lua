local ffi = require "ffi"

local base = require "common.base"
local tunit = require "graphics.textureunit"
require "asset.file"
require "asset.image"

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
void ilG_texture_fromasset(ilG_texture* self, unsigned unit, const struct ilA_file* iface, struct il_base *file);
void ilG_texture_fromimage(ilG_texture *self, unsigned unit, struct ilA_img *img);
void ilG_texture_cubemap(ilG_texture *self, unsigned unit, struct ilA_img *faces[6]);
unsigned int /*GLuint*/ ilG_texture_getRaw(ilG_texture *self, unsigned unit);

]]

base.wrap "il.graphics.texture" {
    struct = "ilG_texture";

    setContext = modules.graphics.ilG_texture_setContext;

    setName = modules.graphics.ilG_texture_setName;

    fromfile = function(self, unit, name)
        return modules.graphics.ilG_texture_fromfile(self, tunit.toUnit(unit), name)
    end;

    fromasset = function(self, unit, asset)
        return modules.graphics.ilG_texture_fromasset(self, tunit.toUnit(unit), nil, asset)
    end;

    fromimage = function(self, unit, img)
        return modules.graphics.ilG_texture_fromimage(self, tunit.toUnit(unit), img);
    end;

    cubemap = function(self, unit, textures)
        local arr = ffi.new("struct ilA_img * [6]", textures)
        for i = 0, 5 do
            if arr[i].width ~= arr[i].height then
                error("Image #"..(i+1).."'s dimensions are not square")
            end
        end
        modules.graphics.ilG_texture_cubemap(self, tunit.toUnit(unit), arr)
    end;

    getRaw = function(self, unit)
        return modules.graphics.ilG_texture_getRaw(self, tunit.toUnit(unit))
    end;
}

return modules.graphics.ilG_texture_type

