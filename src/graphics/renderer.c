#include "renderer.h"

#include <stdbool.h>

#include "graphics/context.h"
#include "common/storage.h"
#include "graphics/material.h"
#include "graphics/drawable3d.h"
#include "graphics/texture.h"
#include "util/array.h"

struct ilG_renderer {
    ilG_context *context;
    ilG_drawable3d *drawable;
    ilG_material *material;
    ilG_texture *texture;
    bool valid;
    il_table storage;
    IL_ARRAY(il_positionable,) positionables;
    char name[64];
};

ilG_renderer *ilG_renderer_new()
{
    ilG_renderer *self = calloc(1, sizeof(ilG_renderer));
    return self;
}

void ilG_renderer_free(ilG_renderer *self)
{
    // TODO: Take ownership of more
    free(self);
}

void ilG_renderer_build(ilG_renderer *self, ilG_context *context)
{
    self->context = context;
    if (ilG_material_link(self->material, context)) {
        return;
    }
    self->valid = 1;
}

void ilG_renderer_draw(ilG_renderer *self)
{
    ilG_context *ctx = self->context;
    ilG_bindable_swap(&ctx->drawableb, (void**)ctx->drawable, self->drawable);
    ilG_bindable_swap(&ctx->materialb, (void**)ctx->material, self->material);
    ilG_bindable_swap(&ctx->textureb,  (void**)ctx->texture,  self->texture);
    unsigned i;
    for (i = 0; i < self->positionables.length; i++) {
        ctx->positionable = &self->positionables.data[i];
        ilG_bindable_action(ctx->materialb, ctx->material);
        ilG_bindable_action(ctx->textureb, ctx->texture);
        ilG_bindable_action(ctx->drawableb, ctx->drawable);
    }
}

const il_table *ilG_renderer_getStorage(const ilG_renderer *self)
{
    return &self->storage;
}

il_table *ilG_renderer_mgetStorage(ilG_renderer *self)
{
    return &self->storage;
}

const char *ilG_renderer_getName(const ilG_renderer *self)
{
    return self->name;
}

void ilG_renderer_setName(ilG_renderer *self, const char *name)
{
    strncpy(self->name, name, 64);
    self->name[63] = 0;
}

void ilG_renderer_setDrawable(ilG_renderer *self, ilG_drawable3d *dr)
{
    self->drawable = dr;
}

void ilG_renderer_setMaterial(ilG_renderer *self, ilG_material *mat)
{
    self->material = mat;
}

void ilG_renderer_setTexture(ilG_renderer *self, ilG_texture *tex)
{
    self->texture = tex;
}

void ilG_renderer_addPositionable(ilG_renderer *self, il_positionable pos)
{
    IL_APPEND(self->positionables, pos);
}

