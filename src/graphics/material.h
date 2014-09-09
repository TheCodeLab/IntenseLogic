#ifndef ILG_MATERIAL_H
#define ILG_MATERIAL_H

#include <stdbool.h>

#include "tgl/tgl.h"
#include "util/ilstring.h"
#include "math/matrix.h"
#include "graphics/arrayattrib.h"
#include "asset/node.h"

struct ilG_context;

typedef struct ilG_material_texunit {
    char *location;
    GLint uniform;
    unsigned long type;
} ilG_material_texunit;

typedef struct ilG_material {
    GLuint program, vertshader, fragshader;
    char *attriblocs[ILG_ARRATTR_NUMATTRS];
    char *fraglocs[ILG_ARRATTR_NUMATTRS];
    IL_ARRAY(ilG_material_texunit,) texunits;
    ilA_file vert, frag;
    char name[64];
} ilG_material;

void ilG_material_init(ilG_material*);
void ilG_material_free(ilG_material*);
void ilG_material_name(ilG_material*, const char *name);
void ilG_material_vertex(ilG_material*, ilA_file file);
bool /*success*/ __attribute__((warn_unused_result))
ilG_material_vertex_file(ilG_material *self, const char *filename, char **error);
void ilG_material_fragment(ilG_material*, ilA_file file);
bool /*success*/ __attribute__((warn_unused_result))
ilG_material_fragment_file(ilG_material *self, const char *filename, char **error);
void ilG_material_arrayAttrib(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_fragData(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_textureUnit(ilG_material*, unsigned long type, const char *location);
GLuint ilG_material_getLoc(ilG_material*, const char *location);
void ilG_material_bind(ilG_material*);
void ilG_material_bindMatrix(ilG_material*, GLuint loc, il_mat m);
bool /*success*/ __attribute__((warn_unused_result))
ilG_material_link(ilG_material*, struct ilG_context *ctx, char **error);

#endif
