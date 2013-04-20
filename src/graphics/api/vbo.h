#ifndef ILG_VBO_H
#define ILG_VBO_H

#include "common/base.h"
#include "math/vector.h"

extern il_type ilG_vbo_type;

typedef struct ilG_vbo ilG_vbo;

#define ilG_vbo_new il_new(&ilG_vbo_type)

void ilG_vbo_setAttributes(ilG_vbo *self, const char *attribs);

void ilG_vbo_beginVertex(ilG_vbo *self);
void ilG_vbo_position   (ilG_vbo *self, il_vec3 position);
void ilG_vbo_texcoord   (ilG_vbo *self, il_vec3 texcoord);
void ilG_vbo_normal     (ilG_vbo *self, il_vec3 normal);
void ilG_vbo_ambient    (ilG_vbo *self, il_vec3 ambient);
void ilG_vbo_diffuse    (ilG_vbo *self, il_vec3 diffuse);
void ilG_vbo_specular   (ilG_vbo *self, il_vec4 specular);

void ilG_vbo_assemble(ilG_vbo *self);

#endif

