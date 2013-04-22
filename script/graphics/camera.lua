local ffi = require "ffi"

local positionable = require "positionable"
local matrix = require "matrix"
local positionable = require "positionable"
local vector3 = require "vector3"

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

