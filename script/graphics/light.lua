local ffi = require "ffi"

require "math.scalar_defs"

local base = require "common.base"
local positionable = require "common.positionable"
local vector3 = require "math.vector3"

ffi.cdef [[

typedef unsigned int GLuint; // TODO: fix this somehow

enum ilG_light_type {
    ILG_POINT,
    ILG_DIRECTIONAL
};

typedef struct ilG_light {
    il_positionable positionable;
    il_vec3 color;
    enum ilG_light_type type;
    GLuint texture; // shadow map
    float radius;
} ilG_light;

extern il_type ilG_light_type;

void ilG_light_add(ilG_light*, struct ilG_context* context);

]]

base.wrap "il.graphics.light" {
    struct = "ilG_light";
    add = modules.graphics.ilG_light_add;
}

return modules.graphics.ilG_light_type

