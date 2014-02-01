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

ilG_light *ilG_light_new();
void ilG_light_free(ilG_light *self);

void ilG_light_setPositionable(ilG_light *self, il_positionable pos);
void ilG_light_add(ilG_light *self, struct ilG_context* context);

]]

local light = {}

function light:add(ctx)
    modules.graphics.ilG_light_add(self, ctx)
end

function light.create()
    return modules.graphics.ilG_light_new()
end

setmetatable(light, {__call = function(self, ...) return light.create(...) end})

ffi.metatype("ilG_light", {
    __index = light
})

return light

