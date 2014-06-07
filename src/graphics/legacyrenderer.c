#include "renderer.h"

#include <stdbool.h>

#include "graphics/context.h"
#include "common/storage.h"
#include "graphics/material.h"
#include "graphics/drawable3d.h"
#include "graphics/tex.h"
#include "util/array.h"
#include "util/log.h"

struct mat {
    int type;
    unsigned loc;
};

struct ilG_legacy {
    ilG_context *context;
    ilG_drawable3d *drawable;
    ilG_material *material;
    IL_ARRAY(ilG_tex,) textures;
    IL_ARRAY(struct mat,) mats;
};

static void legacy_free(void *obj)
{
    ilG_legacy *self = obj;
    il_unref(self->drawable);
    ilG_material_free(self->material);
    free(self);
}

static void legacy_draw(void *obj, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats)
{
    (void)id, (void)objects;
    ilG_legacy *self = obj;
    ilG_context *ctx = self->context;
    ilG_bindable_swap(&ctx->drawableb, (void**)&ctx->drawable, self->drawable);
    ilG_material_bind(self->material);
    for (unsigned i = 0; i < num_mats; i++) {
        for (unsigned j = 0; j < self->mats.length; j++) {
            ilG_material_bindMatrix(self->material, self->mats.data[j].loc, mats[j][i]);
        }
        for (unsigned j = 0; j < self->textures.length; j++) {
            ilG_tex_bind(&self->textures.data[j]);
        }
        ilG_bindable_action(ctx->drawableb, ctx->drawable);
        ilG_testError("Legacy Renderer");
    }
}

bool ilG_legacy_build(void *obj, ilG_rendid id, ilG_context *context, ilG_buildresult *out)
{
    (void)id;
    ilG_legacy *self = obj;
    self->context = context;
    if (ilG_material_link(self->material, context)) {
        return false;
    }
    int *types = malloc(sizeof(int) * self->mats.length);
    for (unsigned i = 0; i < self->mats.length; i++) {
        types[i] = self->mats.data[i].type;
    }
    *out = (ilG_buildresult) {
        .free = legacy_free,
        .update = NULL,
        .draw = legacy_draw,
        .view = NULL,
        .types = types,
        .num_types = self->mats.length,
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

void ilG_legacy_addTexture(ilG_legacy *self, ilG_tex tex)
{
    IL_APPEND(self->textures, tex);
}

void ilG_legacy_addMatrix(ilG_legacy *self, unsigned loc, int type)
{
    struct mat m = (struct mat) {
        type, loc
    };
    IL_APPEND(self->mats, m);
}
