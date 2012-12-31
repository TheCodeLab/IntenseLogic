#ifndef ILG_TEXTURE_H
#define ILG_TEXTURE_H

struct il_positionable;
struct ilG_texture;
struct ilG_context;

typedef void (*ilG_texture_bind_cb)(struct ilG_context*, void*);
typedef void (*ilG_texture_update_cb)(struct ilG_context*, struct il_positionable*, void*);

typedef struct ilG_texture {
    unsigned int id;
    const char *name;
    ilG_texture_bind_cb bind, unbind;
    ilG_texture_update_cb update;
    void *bind_ctx, *update_ctx, *unbind_ctx;
} ilG_texture;

ilG_texture *ilG_texture_default;

struct ilA_asset;

ilG_texture* ilG_texture_fromfile(const char *name);
ilG_texture* ilG_texture_fromasset(struct ilA_asset* asset);

ilG_texture* ilG_texture_fromId(unsigned int id); // tracker.c
void ilG_texture_assignId(ilG_texture*);
void ilG_texture_setId(ilG_texture*, unsigned int id);

#endif

