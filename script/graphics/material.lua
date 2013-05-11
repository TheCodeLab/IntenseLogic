local ffi = require "ffi"

local ilstring = require "util.ilstring"
local base = require "common.base"

ffi.cdef [[

typedef struct ilG_material {
    il_base base;
    unsigned int id;
    /*GLuint*/ unsigned int program, vertshader, fragshader;
    unsigned long long attrs;
    struct ilG_material_config *config;
    int valid;
    struct ilG_context *context;
    char *name;
} ilG_material;

extern il_type ilG_material_type;
extern ilG_material ilG_material_default;

enum ilG_transform {
    ILG_PROJECTION = 1,
    ILG_VIEW = 2,
    ILG_MODEL = 4,
    ILG_INVERSE = 8,
    ILG_TRANSPOSE = 16,
    ILG_VP = ILG_PROJECTION | ILG_VIEW,
    ILG_MVP = ILG_VP | ILG_MODEL,
};

void ilG_material_vertex(ilG_material*, il_string *source);
void ilG_material_fragment(ilG_material*, il_string *source);
void ilG_material_name(ilG_material*, const char* name);
void ilG_material_arrayAttrib(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_fragData(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_textureUnit(ilG_material*, unsigned long type, const char *location);
void ilG_material_matrix(ilG_material*, enum ilG_transform, const char *location);
int /*failure*/ ilG_material_link(ilG_material*, struct ilG_context *ctx);

]]

base.wrap "il.graphics.material" {
    struct = "struct ilG_material";
    default = ffi.C.ilG_material_default;

    vertex = function(self, source)
        ffi.C.ilG_material_vertex(self, ilstring(source))
    end;

    fragment = function(self, source)
        ffi.C.ilG_material_fragment(self, ilstring(source))
    end;

    mtlname = ffi.C.ilG_material_name;

    arrayAttrib = function(self, attribstr, loc)
        local attrib = ({
            position    = 0,
            texcoord    = 1,
            normal      = 2,
            ambient     = 3,
            diffuse     = 4,
            specular    = 5
        })[attribstr] or error "Unknown array attribute"
        ffi.C.ilG_material_arrayAttrib(self, attrib, loc)
    end;

    fragData = function(self, attribstr, loc)
        local attrib = ({
            accumulation   = 0,
            normal         = 1,
            diffuse        = 2,
            specular       = 3
        })[attribstr] or error "Unknown fragment output"
        ffi.C.ilG_material_fragData(self, attrib, loc)
    end;

    textureUnit = function(self, attribstr, loc)
        local attrib;
        if attribstr == "none" then
            attrib = 0
        elseif attribstr:sub(1, 5) == "color" then
            attrib = 1 + tonumber(attribstr:sub(6, -1))
        elseif attribstr:sub(1, 6) == "normal" then
            attrib = 64 + tonumber(attribstr:sub(7, -1))
        elseif attribstr:sub(1, 4) == "bump" then
            attrib = 128 + tonumber(attribstr:sub(5, -1))
        elseif attribstr:sub(1, 7) == "shadow" then
            attrib = 192 + tonumber(attribstr:sub(8, -1))
        elseif attribstr:sub(1, 7) == "height" then
            attrib = 256 + tonumber(attribstr:sub(8, -1))
        else
            error("Unknown texture unit")
        end
        ffi.C.ilG_material_textureUnit(self, attrib, loc)
    end;

    matrix = function(self, t, loc)
        local modes = {
            P = ffi.C.ILG_PROJECTION,
            V = ffi.C.ILG_VIEW,
            M = ffi.C.ILG_MODEL,
            I = ffi.C.ILG_INVERSE,
            T = ffi.C.ILG_TRANSPOSE
        }
        local mode = 0
        for i = 1, #t do
            mode = bit.bor(mode, modes[t:sub(i, i)])
        end
        ffi.C.ilG_material_matrix(self, mode, loc)
    end;

    link = function(self, ctx)
        local res = ffi.C.ilG_material_link(self, ctx)
        if res == 1 then
            error "Failedd to link material"
        end
    end
}

return ffi.C.ilG_material_type;

