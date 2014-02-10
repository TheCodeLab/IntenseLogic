local ffi = require 'ffi'

ffi.cdef [[

typedef struct ilG_renderable {
    void (*free)(void *obj);
    void (*draw)(void *obj);
    void (*build)(void *obj, struct ilG_context *context);
    il_table *(*get_storage)(void *obj);
    bool (*get_complete)(const void *obj);
    void (*add_positionable)(void *obj, il_positionable pos);
    const char *name;
} ilG_renderable;

typedef struct ilG_renderer {
    void *obj;
    const ilG_renderable *vtable;
} ilG_renderer;

extern const ilG_renderable ilG_legacy_renderer;

ilG_renderer ilG_renderer_wrap(void *obj, const ilG_renderable *vtable);
ilG_renderer ilG_renderer_legacy(struct ilG_drawable3d *dr, struct ilG_material *mtl, struct ilG_texture *tex);
void ilG_renderer_free(ilG_renderer self);

void ilG_renderer_build(ilG_renderer *self, struct ilG_context *context);
void ilG_renderer_draw(ilG_renderer *self);

bool ilG_renderer_isComplete(const ilG_renderer *self);
const il_table *ilG_renderer_getStorage(const ilG_renderer *self);
il_table *ilG_renderer_mgetStorage(ilG_renderer *self);
const char *ilG_renderer_getName(const ilG_renderer *self);

void ilG_renderer_addPositionable(ilG_renderer *self, il_positionable pos);

]]

local renderer = {}

function renderer:build(c)
    assert(self ~= nil and c ~= nil)
    modules.graphics.ilG_renderer_build(self, c)
end

function renderer:add(p)
    assert(self ~= nil and p ~= nil)
    modules.graphics.ilG_renderer_addPositionable(self, p)
end

function renderer.create(dr, mtl, tex)
    assert(dr ~= nil and mtl ~= nil and tex ~= nil)
    return modules.graphics.ilG_renderer_legacy(dr, mtl, tex)
end

function renderer:destroy()
    modules.graphics.ilG_renderer_free(self)
end

ffi.metatype("ilG_renderer", {
    __index = function(self, k)
        return modules.graphics.ilG_renderer_mgetStorage(self)[k] or renderer[k]
    end,
    __newindex = function(self, k, v)
        modules.graphics.ilG_renderer_mgetStorage(self)[k] = v
    end
})

setmetatable(renderer, {__call = function(self, ...) return renderer.create(...) end})
return renderer

