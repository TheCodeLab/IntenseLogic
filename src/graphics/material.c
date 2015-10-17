#include "material.h"

#include <string.h>
#include <stdbool.h>

#include "tgl/tgl.h"
#include "asset/node.h"
#include "graphics/arrayattrib.h"
#include "graphics/graphics.h"
#include "graphics/transform.h"
#include "graphics/renderer.h"
#include "util/array.h"
#include "util/log.h"

void ilG_shader_free(ilG_shader *self)
{
    glDeleteShader(self->object);
    ilA_fileclose(&self->file);
}

void ilG_shader_load(ilG_shader* self, ilA_file file, GLenum type)
{
    self->file = file;
    self->type = type;
}

bool ilG_shader_file(ilG_shader *self, const char *filename, GLenum type, char **error)
{
    bool res = ilA_fileopen(&ilG_shaders, &self->file, filename, -1);
    self->type = type;
    if (!res && error) {
        *error = ilA_strerrora(&self->file.err, NULL);
    }
    return res;
}

bool ilG_shader_compile(ilG_shader *self, char **error)
{
    ilA_map map;
    if (!ilA_mapopen(&map, ILA_READ, self->file)) {
        if (error) {
            *error = ilA_strerrora(&map.err, NULL);
        }
        return false;
    }
    char *source = map.data;
    size_t len = map.size;
    GLuint object;
    if (!tgl_make_shader(&object, self->type, source, len, error)) {
        return false;
    }
    self->object = object;
    ilA_unmapfile(&map);
    if (TGL_EXTENSION(KHR_debug)) {
        glObjectLabel(GL_SHADER,
                      object,
                      self->file.namelen,
                      self->file.name);
    }
    return true;
}

void ilG_material_init(ilG_material *mtl)
{
    memset(mtl, 0, sizeof(ilG_material));
}

void ilG_material_free(ilG_material *self)
{
    for (unsigned i = 0; i < self->texunits.length; i++) {
        free(self->texunits.data[i].location);
    }
    IL_FREE(self->texunits);
    for (unsigned i = 0; i < ILG_GBUFFER_NUMATTACHMENTS; i++) {
        free(self->attriblocs[i]);
        free(self->fraglocs[i]);
    }
    glDeleteProgram(self->program);
}

void ilG_material_name(ilG_material *self, const char *name)
{
    strncpy(self->name, name, 63);
}

void ilG_material_arrayAttrib(ilG_material* self, unsigned long attrib, const char *location)
{
    self->attriblocs[attrib] = strdup(location);
}

void ilG_material_fragData(ilG_material* self, unsigned long attrib, const char *location)
{
    self->fraglocs[attrib] = strdup(location);
}

void ilG_material_textureUnit(ilG_material* self, unsigned long unittype, const char *location)
{
    ilG_material_texunit unit = (ilG_material_texunit){
        strdup(location),
        0,
        unittype
    };
    IL_APPEND(self->texunits, unit);
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
    for (i = 0; i < mtl->texunits.length; i++) {
        glUniform1i(mtl->texunits.data[i].uniform, i);
        tgl_check("glUniform1i()");
    }
}

void ilG_material_bindMatrix(ilG_material* self, GLuint loc, il_mat m)
{
    (void)self;
    glUniformMatrix4fv(loc, 1, GL_TRUE, m.data);
}

bool ilG_material_link(ilG_material *self, ilG_shader *vert, ilG_shader *frag, char **error)
{
    tgl_check("Unknown");
    il_log("Building shader \"%s\"", self->name);
    GLuint po = glCreateProgram();
    if (TGL_EXTENSION(KHR_debug) && self->name[0]) {
        glObjectLabel(GL_PROGRAM, po, strlen(self->name), self->name);
    }
    glAttachShader(po, vert->object);
    glAttachShader(po, frag->object);
    tgl_check("glAttachShader");
    unsigned int i;
    for (i = 0; i < ILG_ARRATTR_NUMATTRS; i++) {
        if (self->attriblocs[i]) {
            glBindAttribLocation(po, i, self->attriblocs[i]);
        }
    }
    tgl_check("Error binding array attributes");
    for (i = 0; i < ILG_GBUFFER_NUMATTACHMENTS; i++) {
        if (self->fraglocs[i]) {
            glBindFragDataLocation(po, i, self->fraglocs[i]);
        }
    }
    tgl_check("Error binding fragment outputs");
    if (!tgl_link_program(po, error)) {
        return false;
    }
    for (i = 0; i < self->texunits.length; i++) {
        self->texunits.data[i].uniform = glGetUniformLocation(po, self->texunits.data[i].location);
    }
    tgl_check("Error binding texture units");
    self->program = po;

    return true;
}
