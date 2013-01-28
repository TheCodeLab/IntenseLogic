local ffi = require "ffi";
local drawable = require "drawable"
local material = require "material"
local texture = require "texture"
local camera;
local world; --require "world"

ffi.cdef[[

struct timeval {
    long int tv_sec;
    long int tv_usec;
};

typedef struct ilG_context {
    struct ilG_drawable3d* drawable;
    struct ilG_material* material;
    struct ilG_texture* texture;
    struct ilG_camera* camera;
    struct il_world* world;
    struct timeval tv;
    unsigned *texunits;
    size_t num_texunits;
    size_t num_active;
} ilG_context;

ilG_context* ilG_context_new();

void ilG_context_setActive(ilG_context*);

]]

local context = {};

local function index(t,k)
    world = world or require "world"
    camera = camera or require "camera"
    if k == "drawable" then 
        return drawable.wrap(t.ptr.drawable)
    elseif k == "material" then
        return material.wrap(t.ptr.material)
    elseif k == "texture" then
        return texture.wrap(t.ptr.texture)
    elseif k == "camera" then
        return camera.wrap(t.ptr.camera)
    elseif k == "world" then
        return world.wrap(t.ptr.world)
    end
    return context[k];
end

local function newindex(t, k, v)
    assert(type(v) == "table");
    if k == "world" then
        assert(ffi.istype("struct il_world*", v.ptr), "Attempt to assign non-world to world");
        t.ptr.world = v.ptr;
        return;
    elseif k == "camera" then
        assert(ffi.istype("struct ilG_camera*", v.ptr), "Attempt to assign non-camera to camera");
        t.ptr.camera = v.ptr;
        return;
    end
    error("Invalid key \""..tostring(k).."\" in context");
end

local function wrap(ptr)
    local obj = {};
    obj.ptr = ptr;
    setmetatable(obj, {__index = index, __newindex = newindex});
    return obj;
end
context.wrap = wrap;

function context.create()
    return wrap(ffi.C.ilG_context_new());
end

function context:setActive()
    ffi.C.ilG_context_setActive(self.ptr);
end

setmetatable(context, {__call = context.create})
return context;

