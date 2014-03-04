#include "renderer.h"

#include <stdbool.h>

#include "graphics/context.h"
#include "common/storage.h"
#include "graphics/material.h"
#include "graphics/drawable3d.h"
#include "graphics/tex.h"
#include "util/array.h"
#include "util/log.h"

struct ilG_legacy {
    ilG_context *context;
    ilG_drawable3d *drawable;
    ilG_material *material;
    IL_ARRAY(ilG_tex,) textures;
    bool valid;
    il_table storage;
    IL_ARRAY(il_positionable,) positionables;
};

static void legacy_free(void *obj)
{
    ilG_legacy *self = obj;
    il_unref(self->drawable);
    il_unref(self->material);
    self->valid = false;
    il_table_free(self->storage);
    IL_FREE(self->positionables);
    free(self);
}

static int legacy_build(void *obj, ilG_context *context)
{
    ilG_legacy *self = obj;
    self->context = context;
    if (ilG_material_link(self->material, context)) {
        return 0;
    }
    self->valid = true;
    return 1;
}

static void legacy_draw(void *obj)
{
    ilG_testError("Unknown");
    ilG_legacy *self = obj;
    ilG_context *ctx = self->context;
    ilG_bindable_swap(&ctx->drawableb, (void**)&ctx->drawable, self->drawable);
    ilG_bindable_swap(&ctx->materialb, (void**)&ctx->material, self->material);
    unsigned i;
    for (i = 0; i < self->positionables.length; i++) {
        ctx->positionable = &self->positionables.data[i];
        ilG_bindable_action(ctx->materialb, ctx->material);
        for (unsigned j = 0; j < self->textures.length; j++) {
            ilG_tex_bind(&self->textures.data[i]);
        }
        ilG_bindable_action(ctx->drawableb, ctx->drawable);
        ilG_testError("Legacy Renderer");
    }
}

static bool legacy_get_complete(const void *obj)
{
    const ilG_legacy *self = obj;
    return self->valid;
}

static il_table *legacy_get_storage(void *obj)
{
    ilG_legacy *self = obj;
    return &self->storage;
}

static void legacy_add_positionable(ilG_legacy *self, il_positionable pos)
{
    IL_APPEND(self->positionables, pos);
}

static bool pos_eq(il_positionable p1, il_positionable p2)
{
    return p1.id == p2.id && p1.world == p2.world;
}

static void legacy_del_positionable(ilG_legacy *self, il_positionable pos)
{
    for (unsigned i = 0; i < self->positionables.length-1; i++) {
        if (pos_eq(self->positionables.data[i], pos)) {
            self->positionables.data[i] = self->positionables.data[--self->positionables.length];
            return;
        }
    }
    if (self->positionables.length > 0 && pos_eq(self->positionables.data[self->positionables.length-1], pos)) {
        --self->positionables.length;
        return;
    }
    il_error("Renderer %s<%p> does not own positionable %i[%p]", ilG_legacy_renderer.name, self, pos.id, pos.world);
}

static void legacy_message(void *obj, int type, il_value *v)
{
    switch (type) {
    case 1:
        legacy_add_positionable(obj, *(il_positionable*)il_value_tomvoid(v));
        break;
    case 2:
        legacy_del_positionable(obj, *(il_positionable*)il_value_tomvoid(v));
        break;
    }
}

static void legacy_push_msg(void *obj, int type, il_value v)
{
    ilG_legacy *self = obj;
    if (self->context) {
        ilG_context_message(self->context, ilG_legacy_wrap(self), type, v);
    }  else {
        legacy_message(obj, type, &v);
        il_value_free(v);
    }
}

const ilG_renderable ilG_legacy_renderer = {
    .free = legacy_free,
    .draw = legacy_draw,
    .build = legacy_build,
    .get_storage = legacy_get_storage,
    .get_complete = legacy_get_complete,
    .add_positionable = 1,
    .del_positionable = 2,
    .message = legacy_message,
    .push_msg = legacy_push_msg,
    .name = "Legacy"
};

ilG_legacy *ilG_renderer_legacy(ilG_drawable3d *dr, ilG_material *mtl)
{
    ilG_legacy *self = calloc(1, sizeof(ilG_legacy));
    self->drawable = dr;
    self->material = mtl;
    return self;
}

void ilG_renderer_addTexture(ilG_legacy *self, ilG_tex tex)
{
    IL_APPEND(self->textures, tex);
}

