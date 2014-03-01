#ifndef ILG_RENDERER_H
#define ILG_RENDERER_H

#include "common/world.h"

struct ilG_material;
struct ilG_drawable3d;
struct ilG_context;
struct ilG_renderer;
struct ilG_tex;

typedef struct ilG_renderable {
    void (*free)(void *obj);
    void (*draw)(void *obj);
    int (*build)(void *obj, struct ilG_context *context);
    il_table *(*get_storage)(void *obj);
    bool (*get_complete)(const void *obj);
    void (*add_positionable)(void *obj, il_positionable pos);
    void (*add_renderer)(void *obj, struct ilG_renderer r);
    void (*remove_renderer)(void *obj, struct ilG_renderer r);
    void (*message)(void *obj, int type, il_value v);
    const char *name;
} ilG_renderable;

typedef struct ilG_renderer {
    void *obj;
    const ilG_renderable *vtable;
} ilG_renderer;

typedef struct ilG_legacy ilG_legacy;

extern const ilG_renderable ilG_legacy_renderer;

ilG_legacy *ilG_renderer_legacy(struct ilG_drawable3d *dr, struct ilG_material *mtl);
void ilG_renderer_addTexture(ilG_legacy *self, struct ilG_tex tex);

ilG_renderer ilG_renderer_wrap(void *obj, const ilG_renderable *vtable);
void ilG_renderer_free(ilG_renderer self);

void ilG_renderer_build(ilG_renderer *self, struct ilG_context *context);
void ilG_renderer_draw(ilG_renderer *self);

bool ilG_renderer_isComplete(const ilG_renderer *self);
const il_table *ilG_renderer_getStorage(const ilG_renderer *self);
il_table *ilG_renderer_mgetStorage(ilG_renderer *self);
const char *ilG_renderer_getName(const ilG_renderer *self);

int ilG_renderer_addPositionable(ilG_renderer *self, il_positionable pos);
int ilG_renderer_addRenderer(ilG_renderer *self, ilG_renderer r);

#endif

