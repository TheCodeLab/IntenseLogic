#ifndef ILA_MTL_H
#define ILA_MTL_H

#include "util/uthash.h"

typedef struct ilA_mtl {
    UT_hash_handle hh;
    char *name;
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float transparency;
    struct ilA_img *diffuse_map, *specular_map, *specular_highlight_map;
    struct ilA_mtl *cur;
} ilA_mtl;

ilA_mtl *ilA_mesh_parseMtl(const char *filename, const char *data, size_t length);
void ilA_mtl_free(ilA_mtl *self);

#endif

