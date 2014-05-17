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
};

static void legacy_free(void *obj, ilG_rendid id)
{
    (void)id;
    ilG_legacy *self = obj;
    il_unref(self->drawable);
    il_unref(self->material);
    free(self);
}

static void legacy_draw(void *obj, ilG_rendid id)
{
    ilG_testError("Unknown");
    ilG_legacy *self = obj;
    ilG_context *ctx = self->context;
    ilG_bindable_swap(&ctx->drawableb, (void**)&ctx->drawable, self->drawable);
    ilG_bindable_swap(&ctx->materialb, (void**)&ctx->material, self->material);
    unsigned iter;
    il_positionable *pos;
    while ((pos = ilG_context_iterPositionables(ctx, id, &iter))) {
        ctx->positionable = pos;
        ilG_bindable_action(ctx->materialb, ctx->material);
        for (unsigned j = 0; j < self->textures.length; j++) {
            ilG_tex_bind(&self->textures.data[j]);
        }
        ilG_bindable_action(ctx->drawableb, ctx->drawable);
        ilG_testError("Legacy Renderer");
    }
}

bool ilG_legacy_build(void *obj, ilG_rendid id, ilG_context *context, ilG_renderer *out)
{
    ilG_legacy *self = obj;
    self->context = context;
    if (ilG_material_link(self->material, context)) {
        return false;
    }
    *out = (ilG_renderer) {
        .id = id,
        .free = legacy_free,
        .draw = legacy_draw,
        .obj = self
    };
    return true;
}

ilG_legacy *ilG_legacy_new(ilG_drawable3d *dr, ilG_material *mtl)
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

