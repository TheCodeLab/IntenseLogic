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

typedef void(* ilG_material_onBindFunc)(ilG_material *self, int uniform, void *user);
typedef void(* ilG_material_onPosFunc)(ilG_material *self, struct il_positionable *pos, int uniform, void *user);

extern il_type ilG_material_type;
extern ilG_material ilG_material_default;

enum ilG_transform {
    ILG_PROJECTION  = 0x1,
    ILG_VIEW_R      = 0x2,
    ILG_VIEW_T      = 0x4,
    ILG_MODEL_R     = 0x8,
    ILG_MODEL_T     = 0x10,
    ILG_MODEL_S     = 0x20,
    ILG_INVERSE     = 0x40,
    ILG_TRANSPOSE   = 0x80,
    ILG_VIEW = ILG_VIEW_R | ILG_VIEW_T,
    ILG_VP = ILG_PROJECTION | ILG_VIEW,
    ILG_MODEL = ILG_MODEL_R | ILG_MODEL_T | ILG_MODEL_S,
    ILG_MVP = ILG_VP | ILG_MODEL,
};

void ilG_material_vertex(ilG_material*, il_string *source);
void ilG_material_vertex_file(ilG_material *self, const char *filename);
void ilG_material_fragment(ilG_material*, il_string *source);
void ilG_material_fragment_file(ilG_material *self, const char *filename);
void ilG_material_name(ilG_material*, const char* name);
void ilG_material_arrayAttrib(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_fragData(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_textureUnit(ilG_material*, unsigned long type, const char *location);
void ilG_material_matrix(ilG_material*, enum ilG_transform, const char *location);
void ilG_material_bindFunc(ilG_material*, ilG_material_onBindFunc func, void *user, const char *location);
void ilG_material_posFunc(ilG_material*, ilG_material_onPosFunc func, void *user, const char *location);
int /*failure*/ ilG_material_link(ilG_material*, struct ilG_context *ctx);

]]

base.wrap "il.graphics.material" {
    struct = "struct ilG_material";
    default = modules.graphics.ilG_material_default;

    vertex = function(self, source)
        modules.graphics.ilG_material_vertex(self, ilstring(source))
    end;

    vertex_file = function(self, file)
        modules.graphics.ilG_material_vertex_file(self, file)
    end;

    fragment = function(self, source)
        modules.graphics.ilG_material_fragment(self, ilstring(source))
    end;

    fragment_file = function(self, file)
        modules.graphics.ilG_material_fragment_file(self, file)
    end;

    mtlname = modules.graphics.ilG_material_name;

    arrayAttrib = function(self, attribstr, loc)
        local attrib = ({
            position    = 0,
            texcoord    = 1,
            normal      = 2,
            ambient     = 3,
            diffuse     = 4,
            specular    = 5
        })[attribstr] or error "Unknown array attribute"
        modules.graphics.ilG_material_arrayAttrib(self, attrib, loc)
    end;

    fragData = function(self, attribstr, loc)
        local attrib = ({
            accumulation   = 0,
            normal         = 1,
            diffuse        = 2,
            specular       = 3
        })[attribstr] or error "Unknown fragment output"
        modules.graphics.ilG_material_fragData(self, attrib, loc)
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
        modules.graphics.ilG_material_textureUnit(self, attrib, loc)
    end;

    matrix = function(self, t, loc)
        local modes = {
            P = modules.graphics.ILG_PROJECTION,
            V = modules.graphics.ILG_VIEW,
            M = modules.graphics.ILG_MODEL,
            I = modules.graphics.ILG_INVERSE,
            T = modules.graphics.ILG_TRANSPOSE
        }
        local mode = 0
        for i = 1, #t do
            mode = bit.bor(mode, modes[t:sub(i, i)])
        end
        modules.graphics.ilG_material_matrix(self, mode, loc)
    end;

    bindFunc = function(self, fn, loc)
        modules.graphics.ilG_material_bindFunc(self, fn, nil, loc)
    end;

    posFunc = function(self, fn, loc)
        modules.graphics.ilG_material_posFunc(self, fn, nil, loc)
    end;

    link = function(self, ctx)
        local res = modules.graphics.ilG_material_link(self, ctx)
        if res == 1 then
            error "Failedd to link material"
        end
    end
}

return modules.graphics.ilG_material_type;

