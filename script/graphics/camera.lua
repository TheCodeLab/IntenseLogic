local ffi = require "ffi"

local positionable = require "positionable"
local matrix = require "matrix"
local positionable = require "positionable"
local vector3 = require "vector3"

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
    if k == "positionable" then
        return positionable.wrap(t.ptr.positionable)
    elseif k == "movespeed" then
        return vector3.wrap(t.ptr.movespeed)
    elseif k == "projection_matrix" then
        return matrix.wrap(t.ptr.projection_matrix);
    elseif k == "sensitivity" then
        return t.ptr.sensitivity;
    end
    return camera[k];
end

local function newindex(t, k, v)
    if k == "positionable" then
        assert(type(v) == "table" and ffi.istype(positionable.type, v.ptr), "Attempt to assign non-positionable to positionable")
        t.ptr.positionable = v.ptr;
    elseif k == "movespeed" then
        assert(type(v) == "table" and ffi.istype(vector3.type, v.ptr), "Attempt to assign non-vector to vector3")
        t.ptr.movespeed = v.ptr;
    elseif k == "projection_matrix" then
        assert(type(v) == "table" and ffi.istype(matrix.type, v.ptr), "Attempt to assign non-matrix to matrix")
        t.ptr.projection_matrix = v.ptr;
    elseif k == "sensitivity" then
        assert(type(v) == "number", "Attempt to assign non-number to number");
        t.ptr.sensitivity = v;
    else
        error("Invalid key \""..tostring(k).."\" in camera")
    end
end

local function wrap(ptr)
    local obj = {};
    obj.ptr = ptr;
    setmetatable(obj, {__index = index, __newindex=newindex})
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

