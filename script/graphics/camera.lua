--- Wrapper for camera type
-- Inherits `common.positionable`.
-- @type camera
local ffi           = require "ffi"
local base          = require "common.base"
local positionable  = require "common.positionable"
require "math.scalar_defs"

ffi.cdef [[

typedef struct ilG_camera {
    il_positionable positionable;
    il_vec3 movespeed;
    il_mat projection_matrix;
    float sensitivity;
} ilG_camera;

ilG_camera *ilG_camera_new();
void ilG_camera_free(ilG_camera *self);

void ilG_camera_setPositionable(ilG_camera *self, il_positionable pos);
void ilG_camera_setMatrix(ilG_camera *self, il_mat mat);
void ilG_camera_setMovespeed(ilG_camera* camera, il_vec3 movespeed, float radians_per_pixel);

]]

local camera = {}

function camera.create()
    return modules.graphics.ilG_camera_new()
end

setmetatable(camera, {__call = function(self, ...) return camera.create(...) end})

return camera

