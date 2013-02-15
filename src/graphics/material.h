#ifndef ILG_MATERIAL_H
#define ILG_MATERIAL_H

#include <GL/glew.h>

#include "common/string.h"
#include "graphics/light.h"
#include "graphics/glutil.h"

struct il_positionable;
struct ilG_material;
struct ilG_context;
struct ilA_asset;

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
    struct ilG_material_config *config;
    int valid;
} ilG_material;

ilG_material* ilG_material_default;

ilG_material* ilG_material_new();
void ilG_material_vertex(ilG_material*, il_string source);
void ilG_material_fragment(ilG_material*, il_string source);
void ilG_material_name(ilG_material*, const char* name);
void ilG_material_arrayAttrib(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_fragData(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_textureUnit(ilG_material*, unsigned long type, const char *location);
void ilG_material_matrix(ilG_material*, enum ilG_transform, const char *location);
int /*failure*/ ilG_material_link(ilG_material*);

ilG_material* ilG_material_fromId(unsigned int id); // tracker.c
void ilG_material_assignId(ilG_material*);
void ilG_material_setId(ilG_material*, unsigned int id);

#endif

