--- Wrapper for positionables
-- Uses the IL object system, see `common.base` for more information
-- @type positionable
local ffi           = require "ffi"
local drawable      = require "graphics.drawable"
local material      = require "graphics.material"
local texture       = require "graphics.texture"
local world         = require "common.world"
local context       = require "graphics.context"
local vector3       = require "math.vector3"
local quaternion    = require "math.quaternion"
local base          = require "common.base"

require "math.scalar_defs"
require "util.timeval"

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
    --- Sets a positionable to be rendered in a context
    -- @tparam positionable self The positionable
    -- @tparam context ctx The context
    track = function(self, ctx)
        modules.graphics.ilG_trackPositionable(ctx, self)
    end,
    --- Unsets a positionable to be rendered in a context
    -- @tparam positionable self The positionable
    -- @tparam context ctx The context
    untrack = function(self, ctx)
        modules.graphics.ilG_untrackPositionable(ctx, self)
    end,
    struct = "il_positionable"
}

return modules.common.il_positionable_type

