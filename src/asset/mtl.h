/** @file mtl.h
 * @brief Used for MTL file parsing, as part of Wavefront OBJ loading 
 */
#ifndef ILA_MTL_H
#define ILA_MTL_H

#include "util/uthash.h"

typedef struct ilA_mtl {
    UT_hash_handle hh;
    char *name;
    int id;
    unsigned char ambient[4];
    unsigned char diffuse[4];
    unsigned char specular[4];
    float transparency;
    struct ilA_img *diffuse_map, *specular_map, *specular_highlight_map;
    struct ilA_mtl *cur;
} ilA_mtl;

/** Parses an MTL file */
ilA_mtl *ilA_mesh_parseMtl(ilA_mtl *mtl, const char *filename, const char *data, size_t length);
void ilA_mtl_free(ilA_mtl *self);

#endif

