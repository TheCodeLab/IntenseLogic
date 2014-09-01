#ifndef ILG_MESH_H
#define ILG_MESH_H

#include <stdbool.h>

#include "tgl/gl.h"

struct ilA_mesh;
struct ilG_context;
struct ilA_fs;

typedef struct ilG_mesh {
    GLuint vbo, vao;
    GLint count;
    GLenum type;
    struct ilA_mesh *mesh;
} ilG_mesh;

enum ilG_mesh_attribs {
    ILG_MESH_ERROR,
    ILG_MESH_POS,
    ILG_MESH_TEX,
    ILG_MESH_NORM,
    ILG_MESH_AMBIENT,
    ILG_MESH_DIFFUSE,
    ILG_MESH_SPECULAR
};

bool __attribute__((warn_unused_result)) ilG_mesh_init(ilG_mesh *mesh, const struct ilA_mesh *source);
bool __attribute__((warn_unused_result)) ilG_mesh_fromfile(ilG_mesh *mesh, struct ilA_fs *fs, const char *name);
void ilG_mesh_free(ilG_mesh *mesh);
enum ilG_mesh_attribs ilG_mesh_build(ilG_mesh *mesh, struct ilG_context *context);
void ilG_mesh_bind(ilG_mesh *mesh);
void ilG_mesh_draw(ilG_mesh *mesh);

#endif
