local ffi = require 'ffi'

ffi.cdef [[

typedef struct ilG_renderer ilG_renderer;

ilG_renderer *ilG_renderer_new();
void ilG_renderer_free(ilG_renderer *self);

void ilG_renderer_build(ilG_renderer *self, struct ilG_context *context);
void ilG_renderer_draw(ilG_renderer *self);

const il_table *ilG_renderer_getStorage(const ilG_renderer *self);
il_table *ilG_renderer_mgetStorage(ilG_renderer *self);
const char *ilG_renderer_getName(const ilG_renderer *self);
void ilG_renderer_setName(ilG_renderer *self, const char *name);

void ilG_renderer_setDrawable(ilG_renderer *self, struct ilG_drawable3d *dr);
// unlinked material
void ilG_renderer_setMaterial(ilG_renderer *self, struct ilG_material *mat);
void ilG_renderer_setTexture(ilG_renderer *self, struct ilG_texture *tex);
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

function renderer.create()
    return modules.graphics.ilG_renderer_new()
end

function renderer:destroy()
    modules.graphics.ilG_renderer_free(self)
end

ffi.metatype("ilG_renderer", {
    __index = function(self, k)
        return modules.graphics.ilG_renderer_mgetStorage(self)[k] or renderer[k]
    end,
    __newindex = function(self, k, v)
        if k == 'drawable' then
            modules.graphics.ilG_renderer_setDrawable(self, v)
        elseif k == 'material' then
            modules.graphics.ilG_renderer_setMaterial(self, v)
        elseif k == 'texture' then
            modules.graphics.ilG_renderer_setTexture(self, v)
        else
            modules.graphics.ilG_renderer_mgetStorage(self)[k] = v
        end
    end
})

setmetatable(renderer, {__call = function(self, ...) return renderer.create(...) end})
return renderer

