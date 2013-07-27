#ifndef ILG_MATERIAL_H
#define ILG_MATERIAL_H

#include <GL/glew.h>

#include "util/ilstring.h"
#include "graphics/light.h"
#include "graphics/glutil.h"
#include "graphics/bindable.h"

struct il_positionable;
struct ilG_material;
struct ilG_context;
struct ilA_asset;

typedef struct ilG_material {
    il_base base;
    unsigned int id;
    GLuint program, vertshader, fragshader;
    unsigned long long attrs;
    struct ilG_material_config *config;
    int valid;
    struct ilG_context *context;
    char *name;
} ilG_material;

typedef void (*ilG_material_customDataFunc)(ilG_material *self, GLint uniform, void *user);

extern il_type ilG_material_type;
extern ilG_material ilG_material_default;

#define ilG_material_new() il_new(&ilG_material_type)
void ilG_material_vertex(ilG_material*, il_string *source);
void ilG_material_vertex_file(ilG_material *self, const char *filename);
void ilG_material_fragment(ilG_material*, il_string *source);
void ilG_material_fragment_file(ilG_material *self, const char *filename);
void ilG_material_name(ilG_material*, const char* name);
void ilG_material_arrayAttrib(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_fragData(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_textureUnit(ilG_material*, unsigned long type, const char *location);
void ilG_material_matrix(ilG_material*, enum ilG_transform, const char *location);
void ilG_material_customUniform(ilG_material*, ilG_material_customDataFunc func, void *user, const char *location);
int /*failure*/ ilG_material_link(ilG_material*, struct ilG_context *ctx);

ilG_material* ilG_material_fromId(unsigned int id); // tracker.c
void ilG_material_assignId(ilG_material*);
void ilG_material_setId(ilG_material*, unsigned int id);

#endif

