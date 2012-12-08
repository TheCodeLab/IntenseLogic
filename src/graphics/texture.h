#ifndef ILG_TEXTURE_H
#define ILG_TEXTURE_H

#include <GL/glew.h>

struct ilG_texture;
struct ilG_material;
struct ilG_drawable3d;

typedef void (*ilG_texture_bind_cb)(struct ilG_texture*, void*);
typedef void (*ilG_texture_update_cb)(struct ilG_texture*, 
        struct ilG_drawable3d*, struct ilG_material*, void*);

typedef struct ilG_texture {
    unsigned int id;
    ilG_texture_bind_cb bind, unbind;
    ilG_texture_update_cb update;
    void *bind_ctx, *update_ctx, *unbind_ctx;
} ilG_texture;

ilG_texture* ilG_texture_fromId(unsigned int id); // tracker.c
void ilG_texture_assignId(ilG_texture*);
void ilG_texture_setId(ilG_texture*, unsigned int id);

#endif

