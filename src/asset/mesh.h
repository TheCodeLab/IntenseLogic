/** @file asset/mesh.h
 * @brief Mesh loading and manipulation
 */

#ifndef ILA_MESH_H
#define ILA_MESH_H

#include "asset/node.h"

/** These match up with the OpenGL primitives without creating a dependency on gl.h */
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

/** Attributes for a mesh */
enum ilA_mesh_attrib {
    ILA_MESH_POSITION   = 0x1,
    ILA_MESH_TEXCOORD   = 0x2,
    ILA_MESH_NORMAL     = 0x4,
    ILA_MESH_AMBIENT    = 0x8,
    ILA_MESH_DIFFUSE    = 0x10,
    ILA_MESH_SPECULAR   = 0x20,
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

/** Create a new mesh using a bitvector of array attributes (see graphics/arrayattrib.h) to allocate and a number of vertices */
ilA_mesh *ilA_mesh_new(enum ilA_mesh_attrib attribs, size_t vertices);
/** Copies a mesh */
ilA_mesh *ilA_mesh_copy(const ilA_mesh *mesh);
/** Loads a mesh from a file path */
ilA_mesh *ilA_mesh_loadfile(ilA_fs *fs, const char *path);
/** Loads a mesh from memory */
ilA_mesh *ilA_mesh_loadmem(ilA_fs *fs, const char *filename, const void *data, size_t length);
/** Destroys all data associated with a mesh */
void ilA_mesh_free(ilA_mesh *self);
/** Creates lines out of the position and normal data of a mesh, for debugging purposes. The f paramter is a factor for how long the lines should be */
ilA_mesh *ilA_mesh_debugLines(ilA_mesh *self, float f);

#endif
