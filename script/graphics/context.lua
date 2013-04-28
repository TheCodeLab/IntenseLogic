local ffi       = require "ffi"
local drawable  = require "graphics.drawable"
local material  = require "graphics.material"
local texture   = require "graphics.texture"
local camera
local world --require "world"

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
    struct {
        struct il_positionable* data;
        size_t length;
        size_t capacity;
    } positionables; // tracker.c
    struct {
        struct ilG_light* data;
        size_t length;
        size_t capacity;
    } lights;
    unsigned int /*GLuint*/ fbtextures[5], framebuffer; // depth, accumulation, normal, diffuse, specular
    int width, height;
    struct {
        unsigned int /*GLuint*/ vao, vbo, ibo, lights_ubo, lights_index, mvp_ubo, mvp_index;
        int /*GLint*/ lights_size, mvp_size, lights_offset[3], mvp_offset[1];
        struct ilG_material* material;
        int invalidated;
        int created;
    } lightdata;
    int which;

} ilG_context;

ilG_context* ilG_context_new(int w, int h);

void ilG_context_setActive(ilG_context*);

]]

local context = {};

local function index(t,k)
    world = world or require "common.world"
    camera = camera or require "graphics.camera"
    if k == "drawable" then 
        return drawable.wrap(t.ptr.drawable)
    elseif k == "material" then
        return material.wrap(t.ptr.material)
    elseif k == "texture" then
        return texture.wrap(t.ptr.texture)
    elseif k == "camera" then
        return t.ptr.camera
    elseif k == "world" then
        return t.ptr.world
    end
    return context[k];
end

local function newindex(t, k, v)
    --assert(type(v) == "table");
    if k == "world" then
        assert(ffi.istype("il_world*", v) or ffi.istype("il_base*", v) and v.type and ffi.string(v.type.name)=="il.common.world", "Attempt to assign non-world to world")
        t.ptr.world = v;
        return;
    elseif k == "camera" then
        assert(ffi.istype("ilG_camera*", v) or ffi.istype("il_base*", v) and v.type and ffi.string(v.type.name)=="il.graphics.camera", "Attempt to assign non-camera to camera");
        t.ptr.camera = ffi.cast("ilG_camera*", v);
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

function context.create(w, h)
    return wrap(ffi.C.ilG_context_new(w, h));
end

function context:setActive()
    ffi.C.ilG_context_setActive(self.ptr);
end

setmetatable(context, {__call = function(self, ...) return context.create(...) end})
return context;

