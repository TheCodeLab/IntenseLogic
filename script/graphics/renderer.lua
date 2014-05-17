local ffi = require 'ffi'

ffi.cdef [[

typedef unsigned ilG_rendid;

typedef bool (*ilG_build_fn)(void *obj, ilG_rendid id, struct ilG_context *context, struct ilG_renderer *out);
typedef struct ilG_builder {
    const ilG_build_fn fn;
    void *obj;
} ilG_builder;

typedef struct ilG_handle {
    ilG_rendid id;
    struct ilG_context *context;
} ilG_handle;

typedef struct ilG_rendermanager ilG_rendermanager;

typedef struct ilG_legacy ilG_legacy;
ilG_legacy *ilG_legacy_new(struct ilG_drawable3d *dr, struct ilG_material *mtl);
void ilG_legacy_addTexture(ilG_legacy *self, struct ilG_tex tex);
bool ilG_legacy_build(void *ptr, ilG_rendid id, struct ilG_context *context, struct ilG_renderer *out);

ilG_builder ilG_builder_wrap(void *obj, const ilG_build_fn build);

ilG_handle ilG_build(ilG_builder self, struct ilG_context *context);
void ilG_handle_destroy(ilG_handle self);
bool ilG_handle_ready(ilG_handle self);
il_table *ilG_handle_storage(ilG_handle self);
const char *ilG_handle_getName(ilG_handle self);
void ilG_handle_addPositionable(ilG_handle self, il_positionable pos);
void ilG_handle_delPositionable(ilG_handle self, il_positionable pos);
void ilG_handle_addRenderer(ilG_handle self, ilG_handle node);
void ilG_handle_delRenderer(ilG_handle self, ilG_handle node);
void ilG_handle_addLight(ilG_handle self, ilG_light light);
void ilG_handle_delLight(ilG_handle self, ilG_light light);
void ilG_handle_message(ilG_handle self, int type, il_value v);

// skyboxpass.h

ilG_builder ilG_skybox_builder(struct ilG_tex skytex);

// geometrypass.h

ilG_builder ilG_geometry_builder();

// lightpass.h

ilG_builder ilG_lights_builder();

// transparencypass.h

ilG_builder ilG_transparency_builder();

// outpass.h

ilG_builder ilG_out_builder();

]]

local renderer = {}

-----------------------
-- constructors

function renderer.builder(...)
    if select('#', ...) == 3 then
        return renderer.legacy(...)
    elseif select('#', ...) == 2 then
        return renderer.wrap(...)
    end
    error("Don't know how to handle arguments")
end

function renderer.wrap(ptr, fn)
    assert(fn ~= nil)
    return modules.graphics.ilG_builder_wrap(ptr, fn)
end

function renderer.legacy(dr, mtl)
    assert(dr ~= nil and mtl ~= nil)
    return renderer.wrap(modules.graphics.ilG_legacy_new(dr, mtl), modules.graphics.ilG_legacy_build)
end

function renderer.skybox(tex)
    assert(tex ~= nil)
    return modules.graphics.ilG_skybox_builder(tex)
end

function renderer.geometry()
    return modules.graphics.ilG_geometry_builder()
end

function renderer.lights()
    return modules.graphics.ilG_lights_builder()
end

function renderer.transparency()
    return modules.graphics.ilG_transparency_builder()
end

function renderer.out()
    return modules.graphics.ilG_out_builder()
end

--------------------
-- methods

function renderer:build(c)
    assert(self ~= nil and c ~= nil)
    return modules.graphics.ilG_build(self, c)
end

function renderer:add(o)
    assert(self ~= nil and o ~= nil)
    if ffi.istype('il_positionable', o) then
        modules.graphics.ilG_handle_addPositionable(self, o)
    elseif ffi.istype('ilG_handle', o) then
        modules.graphics.ilG_handle_addRenderer(self, o)
    elseif ffi.istype('ilG_light', o) then
        modules.graphics.ilG_handle_addLight(self, o)
    elseif ffi.istype('ilG_builder', o) then
        error("Argument is a builder - try calling build() first")
    else
        error("Can't handle type "..type(o))
    end
end

function renderer:del(o)
    assert(self ~= nil and o ~= nil)
    if ffi.istype('il_positionable', o) then
        modules.graphics.ilG_handle_delPositionable(self, o)
    elseif ffi.istype('ilG_handle', o) then
        modules.graphics.ilG_handle_delRenderer(self, o)
    elseif ffi.istype('ilG_light', o) then
        modules.graphics.ilG_handle_delLight(self, o)
    else
        error("Can't handle type")
    end
end

function renderer:destroy()
    modules.graphics.ilG_handle_destroy(self)
end

ffi.metatype("ilG_handle", {
    __index = function(self, k)
        local s = modules.graphics.ilG_handle_storage(self)
        return 
           (k == 'complete' and modules.graphics.ilG_handle_ready(self)) 
        or (k == 'name' and ffi.string(modules.graphics.ilG_handle_getName(self)))
        or (s ~= nil and s[k] or false)
        or renderer[k]
    end,
    __newindex = function(self, k, v)
        local s = modules.graphics.ilG_handle_storage(self)
        assert(s ~= nil)
        s[k] = v
    end
})
ffi.metatype("ilG_builder", {
    __index = function(self, k)
        if k == 'build' then
            return renderer.build
        end
    end
})

setmetatable(renderer, {__call = function(self, ...) return renderer.create(...) end})
return renderer

