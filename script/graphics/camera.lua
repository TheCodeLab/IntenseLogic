local ffi = require "ffi"

local positionable = require "positionable"

ffi.cdef [[

typedef float il_Matrix[16];
typedef float il_Vector3[3];

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

