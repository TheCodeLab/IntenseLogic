#ifndef ILG_MATERIAL_H
#define ILG_MATERIAL_H

#include <GL/glew.h>

struct il_positionable;
struct ilG_material;
struct ilG_context;

typedef void (*ilG_material_bind_cb)(struct ilG_material*, void*);
typedef void (*ilG_material_update_cb)(struct ilG_context*, struct il_positionable*, void*);

typedef struct ilG_material {
    unsigned int id;
    const char *name;
    GLuint program, vertshader, fragshader;
    ilG_material_bind_cb bind, unbind;
    ilG_material_update_cb update;
    void *bind_ctx, *update_ctx, *unbind_ctx;
} ilG_material;

ilG_material* ilG_material_default;

ilG_material* ilG_material_fromId(unsigned int id); // tracker.c
void ilG_material_assignId(ilG_material*);
void ilG_material_setId(ilG_material*, unsigned int id);

#endif

