#ifndef ILA_MESH_H
#define ILA_MESH_H

#include "common/base.h"

enum ilA_mesh_primitive {
    ILA_MESH_POINTS,
    ILA_MESH_LINES, 
    ILA_MESH_LINE_STRIP, 
    ILA_MESH_LINE_LOOP, 
    ILA_MESH_LINE_STRIP_ADJACENCY, 
    ILA_MESH_LINES_ADJACENCY, 
    ILA_MESH_TRIANGLES, 
    ILA_MESH_TRIANGLE_STRIP, 
    ILA_MESH_TRIANGLE_FAN,
    ILA_MESH_TRIANGLE_STRIP_ADJACENCY, 
    ILA_MESH_TRIANGLES_ADJACENCY,
    ILA_MESH_PATCHES
};

typedef struct ilA_mesh {
    enum ilA_mesh_primitive mode;
    size_t num_vertices, texcoord_size;
    float (*position)[4];
    float (*texcoord)[4];
    float (*normal)[4];
    unsigned char (*ambient)[4];
    unsigned char (*diffuse)[4];
    unsigned char (*specular)[4];
} ilA_mesh;

void ilA_mesh_free(ilA_mesh *self);

#endif

