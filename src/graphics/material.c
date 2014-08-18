#include "material.h"

#include <GL/glew.h>
#include <string.h>

#include "asset/node.h"
#include "graphics/arrayattrib.h"
#include "graphics/context.h"
#include "graphics/fragdata.h"
#include "graphics/graphics.h"
#include "graphics/transform.h"
#include "tgl/tgl.h"
#include "util/array.h"
#include "util/ilassert.h"
#include "util/log.h"

struct textureunit {
    char *location;
    GLint uniform;
    unsigned long type;
};

struct ilG_material_config {
    IL_ARRAY(struct textureunit,) texunits;
    il_string *vertsource;
    il_string *fragsource;
    char* attriblocs[ILG_ARRATTR_NUMATTRS];
    char* fraglocs[ILG_FRAGDATA_NUMATTRS];
};

char *strdup(const char*);

void ilG_material_init(ilG_material *mtl)
{
    memset(mtl, 0, sizeof(ilG_material));
    mtl->config = calloc(1, sizeof(struct ilG_material_config));
}

void ilG_material_free(ilG_material *self)
{
    IL_FREE(self->config->texunits);
    il_unref(self->config->fragsource);
    il_unref(self->config->vertsource);
    for (unsigned i = 0; i < ILG_FRAGDATA_NUMATTRS; i++) {
        free(self->config->attriblocs[i]);
        free(self->config->fraglocs[i]);
    }
    free(self->config);
    free(self->name);
    glDeleteShader(self->vertshader);
    glDeleteShader(self->fragshader);
    glDeleteProgram(self->program);
}

void ilG_material_vertex(ilG_material* self, il_string *source)
{
    if (self->config->vertsource) {
        il_string_unref(self->config->vertsource);
    }
    self->config->vertsource = il_string_ref(source);
}

void ilG_material_vertex_file(ilG_material *self, const char *filename)
{
    ilA_map map;
    if (!ilA_mapfile(&ilG_shaders, &map, ILA_READ, filename, -1)) {
        ilA_printerror(&map.err);
        return;
    }
    ilG_material_vertex(self, il_string_bin(map.data, map.size));
    ilA_unmapfile(&map);
}

void ilG_material_fragment(ilG_material* self, il_string *source)
{
    if (self->config->fragsource) {
        il_string_unref(self->config->fragsource);
    }
    self->config->fragsource = il_string_ref(source);
}

void ilG_material_fragment_file(ilG_material *self, const char *filename)
{
    ilA_map map;
    if (!ilA_mapfile(&ilG_shaders, &map, ILA_READ, filename, -1)) {
        ilA_printerror(&map.err);
        return;
    }
    ilG_material_fragment(self, il_string_bin(map.data, map.size));
    ilA_unmapfile(&map);
}


void ilG_material_name(ilG_material* self, const char* name)
{
    self->name = strdup(name);
}

void ilG_material_arrayAttrib(ilG_material* self, unsigned long attrib, const char *location)
{
    self->config->attriblocs[attrib] = strdup(location);
}

void ilG_material_fragData(ilG_material* self, unsigned long attrib, const char *location)
{
    self->config->fraglocs[attrib] = strdup(location);
}

void ilG_material_textureUnit(ilG_material* self, unsigned long unittype, const char *location)
{
    struct textureunit unit = (struct textureunit){
        strdup(location),
        0,//glGetUniformLocation(self->program, location),
        unittype
    };
    IL_APPEND(self->config->texunits, unit);
}

GLuint ilG_material_getLoc(ilG_material *self, const char *location)
{
    return glGetUniformLocation(self->program, location);
}

void ilG_material_bind(ilG_material *mtl)
{
    tgl_check("Unknown");
    glUseProgram(mtl->program);
    tgl_check("glUseProgram()");
    unsigned int i;
    for (i = 0; i < mtl->config->texunits.length; i++) {
        glUniform1i(mtl->config->texunits.data[i].uniform, i);
        tgl_check("glUniform1i()");
    }
}

void ilG_material_bindMatrix(ilG_material* self, GLuint loc, il_mat m)
{
    (void)self;
    glUniformMatrix4fv(loc, 1, GL_TRUE, m.data);
}

static void link_shader(ilG_material *self)
{
    tgl_check("Unknown");
    il_log("Building shader \"%s\"", self->name);
    il_string *vs = self->config->vertsource, *fs = self->config->fragsource;
    if (!tgl_make_shader(&self->vertshader, GL_VERTEX_SHADER, vs->data, vs->length)) {
        self->valid = 0;
        return;
    }
    if (!tgl_make_shader(&self->fragshader, GL_FRAGMENT_SHADER, fs->data, fs->length)) {
        self->valid = 0;
        return;
    }
    self->program = glCreateProgram();
    /*if (GLEW_KHR_debug) {
        glObjectLabel(GL_PROGRAM, self->program, strlen(self->name), self->name);
        char buf[1024];
        glGetObjectLabel(GL_PROGRAM, self->program, 1024, NULL, buf);
        printf("Program %u labelled %s\n", self->program, buf);
    }*/
    glAttachShader(self->program, self->vertshader);
    glAttachShader(self->program, self->fragshader);
    tgl_check("glAttachShader");
    unsigned int i;
    for (i = 0; i < ILG_ARRATTR_NUMATTRS; i++) {
        if (self->config->attriblocs[i]) {
            glBindAttribLocation(self->program, i, self->config->attriblocs[i]);
            ILG_SETATTR(self->attrs, i);
        }
    }
    tgl_check("Error binding array attributes");
    for (i = 0; i < ILG_FRAGDATA_NUMATTRS; i++) {
        if (self->config->fraglocs[i]) {
            glBindFragDataLocation(self->program, i, self->config->fraglocs[i]);
        }
    }
    tgl_check("Error binding fragment outputs");
    tgl_link_program(self->program);
    for (i = 0; i < self->config->texunits.length; i++) {
        self->config->texunits.data[i].uniform = glGetUniformLocation(self->program, self->config->texunits.data[i].location);
    }
    tgl_check("Error binding texture units");

    self->valid = 1;
}

int /*failure*/ ilG_material_link(ilG_material* self, ilG_context *ctx)
{
    if (!ctx) {
        il_error("Null context");
    }
    if (!self) {
        il_error("Null material");
    }
    self->context = ctx;
    link_shader(self);
    return 0;
}
