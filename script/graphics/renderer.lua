local ffi = require 'ffi'

ffi.cdef [[

typedef struct ilG_renderable {
    void (*free)(void *obj);
    void (*draw)(void *obj);
    int (*build)(void *obj, struct ilG_context *context);
    il_table *(*get_storage)(void *obj);
    bool (*get_complete)(const void *obj);
    int add_positionable, del_positionable;
    int add_renderer, del_renderer;
    int add_light, del_light;
    void (*message)(void *obj, int type, il_value *v);
    void (*push_msg)(void *obj, int type, il_value v);
    const char *name;
} ilG_renderable;

typedef struct ilG_renderer {
    void *obj;
    const ilG_renderable *vtable;
} ilG_renderer;

typedef struct ilG_legacy ilG_legacy;

extern const ilG_renderable ilG_legacy_renderer;

ilG_renderer ilG_renderer_wrap(void *obj, const ilG_renderable *vtable);
ilG_legacy *ilG_renderer_legacy(struct ilG_drawable3d *dr, struct ilG_material *mtl);
void ilG_renderer_free(ilG_renderer self);

void ilG_renderer_build(ilG_renderer *self, struct ilG_context *context);
void ilG_renderer_draw(ilG_renderer *self);

bool ilG_renderer_isComplete(const ilG_renderer *self);
const il_table *ilG_renderer_getStorage(const ilG_renderer *self);
il_table *ilG_renderer_mgetStorage(ilG_renderer *self);
const char *ilG_renderer_getName(const ilG_renderer *self);

int ilG_renderer_addPositionable(ilG_renderer *self, il_positionable pos);
int ilG_renderer_delPositionable(ilG_renderer *self, il_positionable pos);
int ilG_renderer_addRenderer(ilG_renderer *self, ilG_renderer node);
int ilG_renderer_delRenderer(ilG_renderer *self, ilG_renderer node);
int ilG_renderer_addLight(ilG_renderer *self, struct ilG_light light);
int ilG_renderer_delLight(ilG_renderer *self, struct ilG_light light);

// skyboxpass.h

typedef struct ilG_skybox ilG_skybox;
extern const ilG_renderable ilG_skybox_renderer;
ilG_skybox *ilG_skybox_new(struct ilG_tex skytex);

// geometrypass.h

typedef struct ilG_geometry ilG_geometry;
extern const ilG_renderable ilG_geometry_renderer;
struct ilG_geometry *ilG_geometry_new();

// lightpass.h

typedef struct ilG_lights ilG_lights;
extern const ilG_renderable ilG_lights_renderer;
ilG_lights *ilG_lights_new();
void ilG_lights_add(ilG_lights *self, struct ilG_light light);

// transparencypass.h

typedef struct ilG_transparency ilG_transparency;
extern const ilG_renderable ilG_transparency_renderer;
ilG_transparency *ilG_transparency_new();

// guipass.h

typedef struct ilG_gui ilG_gui;
extern const ilG_renderable ilG_gui_renderer;
ilG_gui *ilG_gui_new(struct ilG_gui_frame *root);

// outpass.h

typedef struct ilG_out ilG_out;
extern const ilG_renderable ilG_out_renderer;
ilG_out *ilG_out_new();

]]

local renderer = {}

-----------------------
-- constructors

function renderer.create(...)
    if select('#', ...) == 3 then
        return renderer.legacy(...)
    elseif select('#', ...) == 2 then
        return renderer.wrap(...)
    end
    error("Don't know how to handle arguments")
end

function renderer.wrap(ptr, vtable)
    assert(vtable ~= nil)
    return modules.graphics.ilG_renderer_wrap(ptr, vtable)
end

function renderer.legacy(dr, mtl)
    assert(dr ~= nil and mtl ~= nil)
    return renderer.wrap(modules.graphics.ilG_renderer_legacy(dr, mtl), modules.graphics.ilG_legacy_renderer)
end

function renderer.skybox(tex)
    assert(tex ~= nil)
    return renderer.wrap(modules.graphics.ilG_skybox_new(tex), modules.graphics.ilG_skybox_renderer)
end

function renderer.geometry()
    return renderer.wrap(modules.graphics.ilG_geometry_new(), modules.graphics.ilG_geometry_renderer)
end

function renderer.lights()
    return renderer.wrap(modules.graphics.ilG_lights_new(), modules.graphics.ilG_lights_renderer)
end

function renderer.transparency()
    return renderer.wrap(modules.graphics.ilG_transparency_new(), modules.graphics.ilG_transparency_renderer)
end

function renderer.gui(root)
    assert(root ~= nil)
    return renderer.wrap(modules.graphics.ilG_gui_new(root), modules.graphics.ilG_gui_renderer)
end

function renderer.out()
    return renderer.wrap(modules.graphics.ilG_out_new(), modules.graphics.ilG_out_renderer)
end

--------------------
-- methods

function renderer:build(c)
    assert(self ~= nil and c ~= nil)
    modules.graphics.ilG_renderer_build(self, c)
end

function renderer:add(o)
    assert(self ~= nil and o ~= nil)
    if ffi.istype('il_positionable', o) then
        modules.graphics.ilG_renderer_addPositionable(self, o)
    elseif ffi.istype('ilG_renderer', o) then
        modules.graphics.ilG_renderer_addRenderer(self, o)
    elseif ffi.istype('ilG_light', o) then
        modules.graphics.ilG_renderer_addLight(self, o)
    else
        error("Can't handle type")
    end
end

function renderer:destroy()
    modules.graphics.ilG_renderer_free(self)
end

ffi.metatype("ilG_renderer", {
    __index = function(self, k)
        local s = modules.graphics.ilG_renderer_mgetStorage(self)
        assert(s ~= nil)
        return 
           (k == 'complete' and modules.graphics.ilG_renderer_isComplete(self)) 
        or (k == 'name' and ffi.string(modules.graphics.ilG_renderer_getName(self)))
        or s[k] 
        or renderer[k]
    end,
    __newindex = function(self, k, v)
        local s = modules.graphics.ilG_renderer_mgetStorage(self)
        assert(s ~= nil)
        s[k] = v
    end
})

setmetatable(renderer, {__call = function(self, ...) return renderer.create(...) end})
return renderer

