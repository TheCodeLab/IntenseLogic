#include "material.h"

#include <GL/glew.h>
#include <string.h>
#include <stdbool.h>

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
    bool file;
    il_string vertsource, fragsource;
    ilA_map vertfile, fragfile;
    char* attriblocs[ILG_ARRATTR_NUMATTRS];
    char* fraglocs[ILG_FRAGDATA_NUMATTRS];
};

char *strdup(const char*);

void ilG_material_init(ilG_material *mtl)
{
    memset(mtl, 0, sizeof(ilG_material));
    mtl->config = calloc(1, sizeof(struct ilG_material_config));
}

static void free_old_vert(struct ilG_material_config *config)
{
    if (config->vertsource.str && config->file) {
        ilA_unmapfile(&config->vertfile);
    } else if (config->vertsource.str) {
        free(config->vertsource.str);
    }
    config->vertsource.str = NULL;
}

static void free_old_frag(struct ilG_material_config *config)
{
    if (config->fragsource.str && config->file) {
        ilA_unmapfile(&config->fragfile);
    } else if (config->fragsource.str) {
        free(config->fragsource.str);
    }
    config->fragsource.str = NULL;
}

void ilG_material_free(ilG_material *self)
{
    IL_FREE(self->config->texunits);
    free_old_vert(self->config);
    free_old_frag(self->config);
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

void ilG_material_vertex(ilG_material* self, il_string source)
{
    free_old_vert(self->config);
    self->config->vertsource = source;
    self->config->file = false;
}

void ilG_material_vertex_file(ilG_material *self, const char *filename)
{
    free_old_vert(self->config);
    ilA_map map;
    if (!ilA_mapfile(&ilG_shaders, &map, ILA_READ, filename, -1)) {
        ilA_printerror(&map.err);
        self->config->vertsource.str = NULL;
        return;
    }
    self->config->vertsource = (il_string){map.data, map.size};
    self->config->vertfile = map;
    self->config->file = true;
}

void ilG_material_fragment(ilG_material* self, il_string source)
{
    free_old_frag(self->config);
    self->config->fragsource = source;
    self->config->file = false;
}

void ilG_material_fragment_file(ilG_material *self, const char *filename)
{
    free_old_frag(self->config);
    ilA_map map;
    if (!ilA_mapfile(&ilG_shaders, &map, ILA_READ, filename, -1)) {
        ilA_printerror(&map.err);
        self->config->fragsource.str = NULL;
        return;
    }
    self->config->fragsource = (il_string){map.data, map.size};
    self->config->fragfile = map;
    self->config->file = true;
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
    il_string vs = self->config->vertsource, fs = self->config->fragsource;
    if (!vs.str || !fs.str) {
        il_error("No source");
        self->valid = 0;
        return;
    }
    if (!tgl_make_shader(&self->vertshader, GL_VERTEX_SHADER, vs.str, vs.len)) {
        self->valid = 0;
        return;
    }
    if (!tgl_make_shader(&self->fragshader, GL_FRAGMENT_SHADER, fs.str, fs.len)) {
        self->valid = 0;
        return;
    }
    self->program = glCreateProgram();
    if (GLEW_KHR_debug && self->name) {
        if (self->config->file) {
            glObjectLabel(GL_SHADER,
                          self->vertshader,
                          self->config->vertfile.namelen,
                          self->config->vertfile.name);
            glObjectLabel(GL_SHADER,
                          self->fragshader,
                          self->config->fragfile.namelen,
                          self->config->fragfile.name);
        }
        glObjectLabel(GL_PROGRAM, self->program, strlen(self->name), self->name);
    }
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
    return !self->valid;
}
