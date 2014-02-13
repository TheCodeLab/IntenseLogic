#include "renderer.h"

#include <stdbool.h>

#include "graphics/context.h"
#include "common/storage.h"
#include "graphics/material.h"
#include "graphics/drawable3d.h"
#include "graphics/tex.h"
#include "util/array.h"

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

void legacy_draw(void *obj)
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

bool legacy_get_complete(const void *obj)
{
    const ilG_legacy *self = obj;
    return self->valid;
}

il_table *legacy_get_storage(void *obj)
{
    ilG_legacy *self = obj;
    return &self->storage;
}

void legacy_add_positionable(void *obj, il_positionable pos)
{
    ilG_legacy *self = obj;
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

