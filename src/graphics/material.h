#ifndef ILG_MATERIAL_H
#define ILG_MATERIAL_H

#include <GL/glew.h>
#include <stdbool.h>

#include "util/ilstring.h"
#include "math/matrix.h"

struct ilG_context;

typedef struct ilG_material {
    GLuint program, vertshader, fragshader;
    unsigned long long attrs;
    struct ilG_material_config *config;
    bool valid;
    struct ilG_context *context;
    char *name;
} ilG_material;

void ilG_material_init(ilG_material*);
void ilG_material_free(ilG_material*);
void ilG_material_vertex(ilG_material*, il_string source);
void ilG_material_vertex_file(ilG_material *self, const char *filename);
void ilG_material_fragment(ilG_material*, il_string source);
void ilG_material_fragment_file(ilG_material *self, const char *filename);
void ilG_material_name(ilG_material*, const char* name);
void ilG_material_arrayAttrib(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_fragData(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_textureUnit(ilG_material*, unsigned long type, const char *location);
GLuint ilG_material_getLoc(ilG_material*, const char *location);
void ilG_material_bind(ilG_material*);
void ilG_material_bindMatrix(ilG_material*, GLuint loc, il_mat m);
int /*failure*/ ilG_material_link(ilG_material*, struct ilG_context *ctx);

#endif
