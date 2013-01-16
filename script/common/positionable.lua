local ffi = require "ffi";
local drawable = require "drawable";
local material = require "material";
local texture = require "texture";
local world = require "world";
local context = require "context";
local vector3 = require "vector3";

require "memory"

ffi.cdef [[

typedef float il_Vector3[3];
typedef float il_Quaternion[4];

typedef struct il_positionable {
  il_Vector3 position;
  il_Quaternion rotation;
  il_Vector3 size;
  il_Vector3 velocity;
  il_GC gc;
  struct timeval last_update;
  struct ilG_drawable3d* drawable;
  struct ilG_material* material;
  struct ilG_texture* texture;
} il_positionable;

il_positionable * il_positionable_new();

void ilG_trackPositionable(struct ilG_context*, struct il_positionable*);
void ilG_untrackPositionable(struct ilG_context*, struct il_positionable*);

]]

local positionable = {};

positionable.type = ffi.typeof "il_positionable*"

local function index(t, k)
    if k == "position" then
        return vector3.wrap(t.ptr.position);
    elseif k == "size" then
        return vector3.wrap(t.ptr.size);
    elseif k == "velocity" then
        return vector3.wrap(t.ptr.velocity);
    elseif k == "drawable" then
        return drawable.wrap(t.ptr.drawable)
    elseif k == "material" then
        return material.wrap(t.ptr.material)
    elseif k == "texture" then
        return texture.wrap(t.ptr.texture)
    end
    return positionable[k];
end

local function newindex(t, k, v)
    assert(type(v) == "table")
    if k == "position" then
        assert(ffi.istype(vector3.type, v.ptr), "Attempt to assign non-vector to vector")
        t.ptr.position = v.ptr;
    elseif k == "size" then
        assert(ffi.istype(vector3.type, v.ptr), "Attempt to assign non-vector to vector")
        t.ptr.size = v.ptr;
    elseif k == "velocity" then
        assert(ffi.istype(vector3.type, v.ptr), "Attempt to assign non-vector to vector")
        t.ptr.velocity = v.ptr;
    elseif k == "drawable" then
        assert(ffi.istype("struct ilG_drawable3d*", v.ptr), "Attempt to assign non-drawable to drawable")
        t.ptr.drawable = v.ptr;
    elseif k == "material" then
        assert(ffi.istype("struct ilG_material*", v.ptr), "Attempt to assign non-material to material")
        t.ptr.material = v.ptr;
    elseif k == "texture" then
        assert(ffi.istype("struct ilG_texture*", v.ptr), "Attempt to assign non-texture to texture")
        t.ptr.texture = v.ptr;
    elseif k == "world" then
        assert(ffi.istype("struct il_world*", v.ptr), "Attempt to assign non-world to world")
        t.ptr.world = v.ptr;
    else
        error("Invalid key \""..tostring(k).."\" in positionable")
    end
end

local function ts(t)
    return "Positionable @ "..tostring(t.world).." "..tostring(t.position)
end

local function wrap(ptr)
    local obj = {};
    obj.ptr = ptr;
    setmetatable(obj, {__index=index, __newindex=newindex, __tostring=ts})
    return obj;
end
positionable.wrap = wrap;

function positionable.create()
    return wrap(ffi.C.il_positionable_new());
end

function positionable:track(ctx)
    assert(type(ctx) == "table", "Expected context");
    assert(ffi.istype("struct ilG_context*", ctx.ptr), "Expected context");
    ffi.C.ilG_trackPositionable(ctx.ptr, self.ptr);
end

function positionable:untrack(ctx)
    assert(type(ctx) == "table", "Expected context");
    assert(ffi.istype("struct ilG_context*", ctx.ptr), "Expected context");
    ffi.C.ilG_untrackPositionable(ctx.ptr, self.ptr);
end


setmetatable(positionable, {__call = function(self, world) return positionable.create(world) end});

return positionable;

