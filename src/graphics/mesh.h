#ifndef ILG_MESH_H
#define ILG_MESH_H

#include "common/base.h"

struct ilG_drawable3d;
struct ilA_mesh;

extern il_type ilG_mesh_type;

struct ilG_drawable3d* ilG_mesh(struct ilA_mesh* self);

struct ilG_drawable3d* ilG_mesh_fromfile(const char *name);

#endif

