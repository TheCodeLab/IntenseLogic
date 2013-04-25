--- Wrapper for camera type
-- Inherits `common.positionable`.
-- @type camera
local ffi = require "ffi"

require "math.scalar_defs"

local positionable  = require "common.positionable"

ffi.cdef [[

typedef struct ilG_camera {
  il_positionable positionable;
  il_vec3 movespeed;
  il_mat projection_matrix;
  float sensitivity;
} ilG_camera;

extern il_type ilG_camera_type;

]]

return ffi.C.ilG_camera_type;

