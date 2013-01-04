local ffi = require "ffi";
local drawable = require "drawable";
local material = require "material";
local texture = require "texture";
local world = require "world";
local context = require "context";

ffi.cdef [[

typedef float il_Vector3[3];
typedef float il_Quaternion[4];

typedef struct il_positionable {
  il_Vector3 position;
  il_Quaternion rotation;
  il_Vector3 size;
  il_Vector3 velocity;
  struct il_world *parent;
  unsigned refs;
  struct timeval last_update;
  struct ilG_drawable3d* drawable;
  struct ilG_material* material;
  struct ilG_texture* texture;
} il_positionable;

il_positionable * il_positionable_new(struct il_world * parent);

void ilG_trackPositionable(struct ilG_context*, struct il_positionable*);
void ilG_untrackPositionable(struct ilG_context*, struct il_positionable*);

]]

local positionable = {};

local function index(t, k)
    if k == "drawable" then
        return drawable.wrap(t.ptr.drawable)
    elseif k == "material" then
        return material.wrap(t.ptr.material)
    elseif k == "texture" then
        return texture.wrap(t.ptr.texture)
    elseif k == "world" then
        return world.wrap(t.ptr.parent)
    end
    return positionable[k];
end

local function newindex(t, k, v)
    assert(type(v) == "table")
    if k == "drawable" then
        assert(ffi.istype("struct ilG_drawable3d*", v.ptr), "Attempt to assign non-drawable to drawable")
        t.ptr.drawable = v.ptr;
        return;
    elseif k == "material" then
        assert(ffi.istype("struct ilG_material*", v.ptr), "Attempt to assign non-material to material")
        t.ptr.material = v.ptr;
        return;
    elseif k == "texture" then
        assert(ffi.istype("struct ilG_texture*", v.ptr), "Attempt to assign non-texture to texture")
        t.ptr.texture = v.ptr;
        return;
    elseif k == "world" then
        assert(ffi.istype("struct il_world*", v.ptr), "Attempt to assign non-world to world")
        t.ptr.world = v.ptr;
        return;
    end
    error("Invalid key \""..tostring(k).."\" in positionable")
end

local function wrap(ptr)
    local obj = {};
    obj.ptr = ptr;
    setmetatable(obj, {__index=index, __newindex=newindex})
    return obj;
end
positionable.wrap = wrap;

function positionable.create(world)
    assert(type(world) == "table", "Expected world");
    assert(ffi.istype("struct il_world*", world.ptr), "Expected world");
    return wrap(ffi.C.il_positionable_new(world.ptr));
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

