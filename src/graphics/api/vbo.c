#include "vbo.h"

#include <GL/glew.h>
#include <stdlib.h>

#include "util/array.h"
#include "graphics/arrayattrib.h"

struct ilG_vbo {
    il_base base;
    GLuint vbo;
    unsigned attribs;
    int assembled;
    struct vertex *current;
    IL_ARRAY(struct vertex,) vertices;
};

il_base *vbo_create(il_type *type)
{
    ilG_vbo *vbo = calloc(1, sizeof(ilG_vbo));
    return &vbo->base;
}

il_type ilG_vbo_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .create = &vbo_create,
    .name = "VBO",
    .registry = NULL
};

struct vertex {
    il_vec3 position;
    il_vec3 texcoord;
    il_vec3 normal;
    il_vec3 ambient;
    il_vec3 diffuse;
    il_vec4 specular;
};

void ilG_vbo_setAttributes(ilG_vbo *self, const char *attribs)
{
    int i;
    for (i = 0; i < strlen(attribs); i++) {
        switch (attribs[i]) {
#define ATTRIB_CASE(c, v) case c: self->attribs |= (1<<v); break
            ATTRIB_CASE('p', ILG_ARRATTR_POSITION);
            ATTRIB_CASE('t', ILG_ARRATTR_TEXCOORD);
            ATTRIB_CASE('n', ILG_ARRATTR_NORMAL);
            ATTRIB_CASE('a', ILG_ARRATTR_AMBIENT);
            ATTRIB_CASE('d', ILG_ARRATTR_DIFFUSE);
            ATTRIB_CASE('s', ILG_ARRATTR_SPECULAR);
#undef ATTRIB_CASE
        }
    }
}

void ilG_vbo_beginVertex(ilG_vbo *self)
{
    self->vertices.length++;
    while (self->vertices.length > self->vertices.capacity) {
        IL_RESIZE(self->vertices);
    }
    self->current = self->vertices.data[self->vertices.length-1];
}

void ilG_vbo_position(ilG_vbo *self, il_vec3 position)
{
    self->current->position = il_vec3_copy(position);
}

void ilG_vbo_texcoord(ilG_vbo *self, il_vec3 texcoord)
{
    self->current->texcoord = il_vec3_copy(texcoord);
}

void ilG_vbo_normal(ilG_vbo *self, il_vec3 normal)
{
    self->current->normal = il_vec3_copy(normal);
}

void ilG_vbo_ambient(ilG_vbo *self, il_vec3 ambient)
{
    self->current->ambient = il_vec3_copy(ambient);
}

void ilG_vbo_diffuse(ilG_vbo *self, il_vec3 diffuse)
{
    self->current->diffuse = il_vec3_copy(diffuse);
}

void ilG_vbo_specular(ilG_vbo *self, il_vec4 specular)
{
    self->current->specular = il_vec4_copy(specular);
}

void ilG_vbo_assemble(ilG_vbo *self)
{
    size_t per_vertex = 
        (
            ILG_TESTATTR(self->attribs, ILG_ARRATTR_POSITION) +
            ILG_TESTATTR(self->attribs, ILG_ARRATTR_TEXCOORD) +
            ILG_TESTATTR(self->attribs, ILG_ARRATTR_NORMAL) +
            ILG_TESTATTR(self->attribs, ILG_ARRATTR_AMBIENT) +
            ILG_TESTATTR(self->attribs, ILG_ARRATTR_DIFFUSE)
        ) * sizeof(float) * 3 +
        ILG_TESTATTR(self->attribs, ILG_ARRATTR_SPECULAR) * sizeof(float) * 4;
    size_t num_vertex = self->vertices.length;
    float *buf = calloc(num_vertex, per_vertex), *ptr = buf;
    int i;
    for (i = 0; i < self->vertices.length; i++) {
        struct vertex *v = self->vertices.data + i;
        if (ILG_TESTATTR(self->attribs, ILG_ARRATTR_POSITION)) {
            *ptr++ = v->position[0];
            *ptr++ = v->position[1];
            *ptr++ = v->position[2];
        }
        if (ILG_TESTATTR(self->attribs, ILG_ARRATTR_TEXCOORD)) {
            *ptr++ = v->texcoord[0];
            *ptr++ = v->texcoord[1];
            *ptr++ = v->texcoord[2];
        }
        if (ILG_TESTATTR(self->attribs, ILG_ARRATTR_NORMAL)) {
            *ptr++ = v->normal[0];
            *ptr++ = v->normal[1];
            *ptr++ = v->normal[2];
        }
        if (ILG_TESTATTR(self->attribs, ILG_ARRATTR_AMBIENT)) {
            *ptr++ = v->ambient[0];
            *ptr++ = v->ambient[1];
            *ptr++ = v->ambient[2];
        }
        if (ILG_TESTATTR(self->attribs, ILG_ARRATTR_DIFFUSE)) {
            *ptr++ = v->diffuse[0];
            *ptr++ = v->diffuse[1];
            *ptr++ = v->diffuse[2];
        }
        if (ILG_TESTATTR(self->attribs, ILG_ARRATTR_SPECULAR)) {
            *ptr++ = v->specular[0];
            *ptr++ = v->specular[1];
            *ptr++ = v->specular[2];
            *ptr++ = v->specular[3];
        }
    }
    glGenBuffers(1, &self->vbo);
}

