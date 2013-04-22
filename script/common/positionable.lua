local ffi = require "ffi"
local drawable = require "drawable"
local material = require "material"
local texture = require "texture"
local world = require "world"
local context = require "context"
local vector3 = require "vector3"
local quaternion = require "quaternion"
local base = require "base"

require "scalar_defs"

ffi.cdef [[

typedef struct il_positionable {
    il_base base;
    il_vec3 position;
    il_quat rotation;
    il_vec3 size;
    il_vec3 velocity;
    struct timeval last_update;
    struct ilG_drawable3d* drawable;
    struct ilG_material* material;
    struct ilG_texture* texture;
} il_positionable;

extern il_type il_positionable_type;

void ilG_trackPositionable(struct ilG_context*, struct il_positionable*);
void ilG_untrackPositionable(struct ilG_context*, struct il_positionable*);

]]

base.wrap "il.common.positionable" {
    track = function(self, ctx)
        ffi.C.ilG_trackPositionable(ctx, self)
    end,
    untrack = function(self, ctx)
        ffi.C.ilG_untrackPositionable(ctx, self)
    end,
    struct = "il_positionable"
}

return ffi.C.il_positionable_type

