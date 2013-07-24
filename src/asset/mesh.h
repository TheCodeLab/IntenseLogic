/** @file mesh.h
 * @brief Mesh loading and manipulation
 */

#ifndef ILA_MESH_H
#define ILA_MESH_H

#include "common/base.h"
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

/** Loads a mesh from a file */
ilA_mesh *ilA_mesh_load(il_base *file, const ilA_file *iface);
/** Loads a mesh from a file path */
ilA_mesh *ilA_mesh_loadfile(const char *path);
/** Loads a mesh from memory */
ilA_mesh *ilA_mesh_loadmem(const char *filename, const void *data, size_t length);
/** Destroys all data associated with a mesh */
void ilA_mesh_free(ilA_mesh *self);
/** Creates lines out of the position and normal data of a mesh, for debugging purposes. The f paramter is a factor for how long the lines should be */
ilA_mesh *ilA_mesh_debugLines(ilA_mesh *self, float f);

#endif

