local ffi = require "ffi"

local ilstring = require "util.ilstring"

ffi.cdef [[

struct ilG_material* ilG_material_default;

enum ilG_transform {
    ILG_PROJECTION = 1,
    ILG_VIEW = 2,
    ILG_MODEL = 4,
    ILG_INVERSE = 8,
    ILG_VP = ILG_PROJECTION | ILG_VIEW,
    ILG_MVP = ILG_VP | ILG_MODEL,
};

struct ilG_material* ilG_material_new();
void ilG_material_vertex(struct ilG_material*, il_string *source);
void ilG_material_fragment(struct ilG_material*, il_string *source);
void ilG_material_name(struct ilG_material*, const char* name);
void ilG_material_arrayAttrib(struct ilG_material*, unsigned long attrib, const char *location);
void ilG_material_fragData(struct ilG_material*, unsigned long attrib, const char *location);
void ilG_material_textureUnit(struct ilG_material*, unsigned long type, const char *location);
void ilG_material_matrix(struct ilG_material*, enum ilG_transform, const char *location);
int /*failure*/ ilG_material_link(struct ilG_material*);

]]

local material = {};

local function wrap(ptr)
    local obj = {}
    obj.ptr = ptr;
    setmetatable(obj, {__index=material})
    return obj;
end
material.wrap = wrap;

material.default = wrap(ffi.C.ilG_material_default);

function material.create()
    return wrap(ffi.C.ilG_material_new())
end

function material:vertex(source)
    assert(type(source) == "string", "Expected string")
    ffi.C.ilG_material_vertex(self.ptr, ilstring(source))
end

function material:fragment(source)
    assert(type(source) == "string", "Expected string")
    ffi.C.ilG_material_fragment(self.ptr, ilstring(source))
end

function material:name(name)
    assert(type(name) == "string", "Expected string")
    ffi.C.ilG_material_name(self.ptr, name)
end

function material:arrayAttrib(attribstr, loc)
    local attrib = ({
        position    = 0,
        texcoord    = 1,
        normal      = 2,
        ambient     = 3,
        diffuse     = 4,
        specular    = 5
    })[attribstr] or error "Unknown array attribute"
    ffi.C.ilG_material_arrayAttrib(self.ptr, attrib, loc)
end

function material:fragData(attribstr, loc)
    local attrib = ({
        accumulation   = 0,
        normal         = 1,
        diffuse        = 2,
        specular       = 3
    })[attribstr] or error "Unknown fragment output"
    ffi.C.ilG_material_fragData(self.ptr, attrib, loc)
end

function material:textureUnit(attribstr, loc)
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
    ffi.C.ilG_material_textureUnit(self.ptr, attrib, loc)
end

function material:matrix(t, loc)
    local modes = {
        P = ffi.C.ILG_PROJECTION,
        V = ffi.C.ILG_VIEW,
        M = ffi.C.ILG_MODEL,
        I = ffi.C.ILG_INVERSE
    }
    local mode = 0
    for i = 1, #t do
        mode = bit.bor(mode, modes[t:sub(i, i)])
    end
    ffi.C.ilG_material_matrix(self.ptr, mode, loc)
end

function material:link()
    local res = ffi.C.ilG_material_link(self.ptr)
    if res == 1 then
        error "Failedd to link material"
    end
end

setmetatable(material, {__call = function(self, ...) return material.create(...) end})

return material;

