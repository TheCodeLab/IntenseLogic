#ifndef ILG_MATERIAL_H
#define ILG_MATERIAL_H

#include <GL/glew.h>

#include "common/string.h"
#include "graphics/light.h"

struct il_positionable;
struct ilG_material;
struct ilG_context;

typedef void (*ilG_material_bind_cb)(struct ilG_context*, void*);
typedef void (*ilG_material_update_cb)(struct ilG_context*, struct il_positionable*, void*);

typedef struct ilG_material {
    unsigned int id;
    const char *name;
    GLuint program, vertshader, fragshader;
    ilG_material_bind_cb bind, unbind;
    ilG_material_update_cb update;
    void *bind_ctx, *update_ctx, *unbind_ctx;
    unsigned long long attrs;
    ilG_phong phong;
} ilG_material;

ilG_material* ilG_material_default;

ilG_material* ilG_material_new(il_string vertsource, il_string fragsource, 
    const char *name, const char *position, const char *texcoord,
    const char *normal, const char *mvp, const char **unitlocs, 
    unsigned long *unittypes, const char *normalOut, const char *ambient, 
    const char *diffuse, const char *specular, const char *phong);

ilG_material* ilG_material_fromId(unsigned int id); // tracker.c
void ilG_material_assignId(ilG_material*);
void ilG_material_setId(ilG_material*, unsigned int id);

#endif

