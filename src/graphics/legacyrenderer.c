#include "renderer.h"

#include <stdbool.h>

#include "graphics/context.h"
#include "common/storage.h"
#include "graphics/material.h"
#include "graphics/drawable3d.h"
#include "graphics/texture.h"
#include "util/array.h"

typedef struct ilG_legacyrenderer {
    ilG_context *context;
    ilG_drawable3d *drawable;
    ilG_material *material;
    ilG_texture *texture;
    bool valid;
    il_table storage;
    IL_ARRAY(il_positionable,) positionables;
} ilG_legacyrenderer;

static void legacy_free(void *obj)
{
    ilG_legacyrenderer *self = obj;
    il_unref(self->drawable);
    il_unref(self->material);
    il_unref(self->texture);
    self->valid = false;
    il_table_free(self->storage);
    IL_FREE(self->positionables);
    free(self);
}

static int legacy_build(void *obj, ilG_context *context)
{
    ilG_legacyrenderer *self = obj;
    self->context = context;
    if (ilG_material_link(self->material, context)) {
        return 0;
    }
    self->valid = true;
    return 1;
}

void legacy_draw(void *obj)
{
    ilG_testError("Unknown");
    ilG_legacyrenderer *self = obj;
    ilG_context *ctx = self->context;
    ilG_bindable_swap(&ctx->drawableb, (void**)&ctx->drawable, self->drawable);
    ilG_bindable_swap(&ctx->materialb, (void**)&ctx->material, self->material);
    ilG_bindable_swap(&ctx->textureb,  (void**)&ctx->texture,  self->texture);
    unsigned i;
    for (i = 0; i < self->positionables.length; i++) {
        ctx->positionable = &self->positionables.data[i];
        ilG_bindable_action(ctx->materialb, ctx->material);
        ilG_bindable_action(ctx->textureb, ctx->texture);
        ilG_bindable_action(ctx->drawableb, ctx->drawable);
        ilG_testError("Legacy Renderer");
    }
}

bool legacy_get_complete(const void *obj)
{
    const ilG_legacyrenderer *self = obj;
    return self->valid;
}

il_table *legacy_get_storage(void *obj)
{
    ilG_legacyrenderer *self = obj;
    return &self->storage;
}

void legacy_add_positionable(void *obj, il_positionable pos)
{
    ilG_legacyrenderer *self = obj;
    IL_APPEND(self->positionables, pos);
}

const ilG_renderable ilG_legacy_renderer = {
    .free = legacy_free,
    .draw = legacy_draw,
    .build = legacy_build,
    .get_storage = legacy_get_storage,
    .get_complete = legacy_get_complete,
    .add_positionable = legacy_add_positionable,
    .name = "Legacy"
};

ilG_renderer ilG_renderer_legacy(ilG_drawable3d *dr, ilG_material *mtl, ilG_texture *tex)
{
    ilG_legacyrenderer *self = calloc(1, sizeof(ilG_legacyrenderer));
    self->drawable = dr;
    self->material = mtl;
    self->texture = tex;
    return ilG_renderer_wrap(self, &ilG_legacy_renderer);
}

