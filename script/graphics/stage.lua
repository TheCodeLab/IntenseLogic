local ffi = require "ffi"

require 'graphics.renderer'

ffi.cdef [[

typedef struct ilG_stagable {
    void (*run)(void *obj);
    int /*success*/ (*track)(void *obj, struct ilG_renderer *r);
    const char *name;
} ilG_stagable;

typedef struct ilG_stage {
    void *obj;
    const ilG_stagable *stagable;
} ilG_stage;

ilG_stage ilG_stage_new(void *obj, const ilG_stagable *stagable);
int ilG_stage_track(ilG_stage self, ilG_renderer renderer);
const char *ilG_stage_getName(ilG_stage self);

// skyboxpass.h

typedef struct ilG_skybox ilG_skybox;

extern const ilG_stagable ilG_skybox_stage;

ilG_skybox *ilG_skybox_new(struct ilG_context *context, struct ilG_texture *skytex);

// geometrypass.h

typedef struct ilG_geometry ilG_geometry;

extern const ilG_stagable ilG_geometry_stage;

struct ilG_geometry *ilG_geometry_new(struct ilG_context *context);

// lightpass.h

typedef struct ilG_lights ilG_lights;

extern const ilG_stagable ilG_lights_stage;

ilG_lights *ilG_lights_new(struct ilG_context *context);

// transparencypass.h

typedef struct ilG_transparency ilG_transparency;

extern const ilG_stagable ilG_transparency_stage;

ilG_transparency *ilG_transparency_new(struct ilG_context *context);

// guipass.h

typedef struct ilG_gui ilG_gui;

extern const ilG_stagable ilG_gui_stage;

ilG_gui *ilG_gui_new(struct ilG_context *context, struct ilG_gui_frame *root);

// outpass.h

typedef struct ilG_out ilG_out;

extern const ilG_stagable ilG_out_stage;

ilG_out *ilG_out_new(struct ilG_context *context);

]]

local stage = {}

function stage.wrap(ptr, vtable)
    assert(vtable ~= nil)
    return modules.graphics.ilG_stage_new(ptr, vtable)
end

function stage.skybox(ctx, tex)
    assert(ctx ~= nil and tex ~= nil)
    return stage.wrap(modules.graphics.ilG_skybox_new(ctx, tex), modules.graphics.ilG_skybox_stage)
end

function stage.geometry(ctx)
    assert(ctx ~= nil)
    return stage.wrap(modules.graphics.ilG_geometry_new(ctx), modules.graphics.ilG_geometry_stage)
end

function stage.lights(ctx)
    assert(ctx ~= nil)
    return stage.wrap(modules.graphics.ilG_lights_new(ctx), modules.graphics.ilG_lights_stage)
end

function stage.transparency(ctx)
    assert(ctx ~= nil)
    return stage.wrap(modules.graphics.ilG_transparency_new(ctx), modules.graphics.ilG_transparency_stage)
end

function stage.gui(ctx, root)
    assert(ctx ~= nil and root ~= nil)
    return stage.wrap(modules.graphics.ilG_gui_new(ctx, root), modules.graphics.ilG_gui_stage)
end

function stage.out(ctx)
    assert(ctx ~= nil)
    return stage.wrap(modules.graphics.ilG_out_new(ctx), modules.graphics.ilG_out_stage)
end

function stage:track(r)
    assert(self ~= nil and r ~= nil)
    assert(1 == tonumber(modules.graphics.ilG_stage_track(self, r)), "ilG_stage_track")
end

ffi.metatype("ilG_stage", {
    __index = function(self, k)
        if k == 'name' then
            return ffi.string(modules.graphics.ilG_stage_getName(self))
        end
        return stage[k]
    end
})

return stage

