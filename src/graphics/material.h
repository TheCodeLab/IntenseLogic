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

typedef struct ilG_shader {
    GLuint object;
    ilA_file file;
} ilG_shader;

typedef struct ilG_material {
    GLuint program;
    unsigned vert, frag;
    char *attriblocs[ILG_ARRATTR_NUMATTRS];
    char *fraglocs[ILG_ARRATTR_NUMATTRS];
    IL_ARRAY(ilG_material_texunit,) texunits;
    char name[64];
} ilG_material;

#define ilG_shader_init(s) memset(s, 0, sizeof(ilG_shader))
void ilG_shader_free(ilG_shader*);
void ilG_shader_load(ilG_shader*, ilA_file file);
bool /*success*/ __attribute__((warn_unused_result))
ilG_shader_file(ilG_shader*, const char *filename, char **error);
bool ilG_shader_compile(ilG_shader*, GLenum type, char **error);

void ilG_material_init(ilG_material*);
void ilG_material_free(ilG_material*);
void ilG_material_name(ilG_material*, const char *name);
void ilG_material_arrayAttrib(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_fragData(ilG_material*, unsigned long attrib, const char *location);
void ilG_material_textureUnit(ilG_material*, unsigned long type, const char *location);
GLuint ilG_material_getLoc(ilG_material*, const char *location);
void ilG_material_bind(ilG_material*);
void ilG_material_bindMatrix(ilG_material*, GLuint loc, il_mat m);
bool /*success*/ __attribute__((warn_unused_result))
ilG_material_link(ilG_material*, ilG_shader *vert, ilG_shader *frag, char **error);

#endif
