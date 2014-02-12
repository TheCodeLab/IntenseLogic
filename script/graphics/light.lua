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

void ilG_light_setPositionable(ilG_light *self, il_positionable pos);

void free(void*);

]]

local light = {}

function light.create()
    return ffi.new('ilG_light')
end

setmetatable(light, {__call = function(self, ...) return light.create(...) end})

ffi.metatype("ilG_light", {
    __index = light
})

return light

