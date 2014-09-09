#include "material.h"

#include <string.h>
#include <stdbool.h>

#include "tgl/tgl.h"
#include "asset/node.h"
#include "graphics/arrayattrib.h"
#include "graphics/context.h"
#include "graphics/fragdata.h"
#include "graphics/graphics.h"
#include "graphics/transform.h"
#include "util/array.h"
#include "util/log.h"

char *strdup(const char*);

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
    for (unsigned i = 0; i < ILG_FRAGDATA_NUMATTRS; i++) {
        free(self->attriblocs[i]);
        free(self->fraglocs[i]);
    }
    glDeleteShader(self->vertshader);
    glDeleteShader(self->fragshader);
    glDeleteProgram(self->program);
    ilA_fileclose(&self->vert);
    ilA_fileclose(&self->frag);
}

void ilG_material_name(ilG_material *self, const char *name)
{
    strncpy(self->name, name, 63);
}

void ilG_material_vertex(ilG_material* self, ilA_file file)
{
    self->vert = file;
}

bool ilG_material_vertex_file(ilG_material *self, const char *filename, char **error)
{
    bool res = ilA_fileopen(&ilG_shaders, &self->vert, filename, -1);
    if (!res && error) {
        *error = ilA_strerrora(&self->vert.err, NULL);
    }
    return res;
}

void ilG_material_fragment(ilG_material* self, ilA_file file)
{
    self->frag = file;
}

bool ilG_material_fragment_file(ilG_material *self, const char *filename, char **error)
{
    bool res = ilA_fileopen(&ilG_shaders, &self->frag, filename, -1);
    if (!res && error) {
        *error = ilA_strerrora(&self->frag.err, NULL);
    }
    return res;
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

bool ilG_material_link(ilG_material *self, ilG_context *context, char **error)
{
    (void)context;
    tgl_check("Unknown");
    il_log("Building shader \"%s\"", self->name);
    ilA_map vert, frag;
    if (!ilA_mapopen(&vert, ILA_READ, self->vert)) {
        if (error) {
            *error = ilA_strerrora(&vert.err, NULL);
        }
        return false;
    }
    if (!ilA_mapopen(&frag, ILA_READ, self->frag)) {
        if (error) {
            *error = ilA_strerrora(&frag.err, NULL);
        }
        return false;
    }
    GLuint vo, fo, po;
    char *vs = vert.data, *fs = frag.data;
    size_t vl = vert.size, fl = frag.size;
    if (!tgl_make_shader(&vo, GL_VERTEX_SHADER, vs, vl, error)) {
        return false;
    }
    if (!tgl_make_shader(&fo, GL_FRAGMENT_SHADER, fs, fl, error)) {
        return false;
    }
    ilA_unmapfile(&vert);
    ilA_unmapfile(&frag);
    po = glCreateProgram();
    if (TGL_EXTENSION(KHR_debug) && self->name) {
        glObjectLabel(GL_SHADER,
                      vo,
                      self->vert.namelen,
                      self->vert.name);
        glObjectLabel(GL_SHADER,
                      fo,
                      self->frag.namelen,
                      self->frag.name);
        glObjectLabel(GL_PROGRAM, po, strlen(self->name), self->name);
    }
    glAttachShader(po, vo);
    glAttachShader(po, fo);
    tgl_check("glAttachShader");
    unsigned int i;
    for (i = 0; i < ILG_ARRATTR_NUMATTRS; i++) {
        if (self->attriblocs[i]) {
            glBindAttribLocation(po, i, self->attriblocs[i]);
        }
    }
    tgl_check("Error binding array attributes");
    for (i = 0; i < ILG_FRAGDATA_NUMATTRS; i++) {
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
    self->vertshader = vo;
    self->fragshader = fo;
    self->program = po;

    return true;
}
