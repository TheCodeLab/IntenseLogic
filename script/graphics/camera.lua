local ffi = require "ffi"

local positionable = require "positionable"
local matrix = require "matrix"

ffi.cdef [[

typedef struct ilG_camera {
  struct il_positionable* positionable;
  il_Vector3 movespeed;
  il_Matrix projection_matrix;
  float sensitivity;
  unsigned refs;
} ilG_camera;

ilG_camera* ilG_camera_new(struct il_positionable * parent);

void ilG_camera_setEgoCamKeyHandlers(ilG_camera* camera, struct il_keymap * keymap);

]]

local camera = {}

local function index(t, k)
    if k == "projection_matrix" then
        return matrix.wrap(t.ptr.projection_matrix);
    end
    return camera[k];
end

local function wrap(ptr)
    local obj = {};
    obj.ptr = ptr;
    setmetatable(obj, {__index = camera})
    return obj
end
camera.wrap = wrap

function camera.create(pos)
    --if not pos then pos = positionable() end
    assert(type(pos) == "table", "Expected positionable")
    assert(ffi.istype("struct il_positionable*", pos.ptr), "Expected positionable")
    return wrap(ffi.C.ilG_camera_new(pos.ptr))
end

setmetatable(camera, {__call = function(self, pos) return camera.create(pos) end})
return camera;

