#ifndef ILG_RENDERER_H
#define ILG_RENDERER_H

#include "common/world.h"

struct ilG_material;
struct ilG_drawable3d;
struct ilG_texture;
struct ilG_context;

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

#endif

