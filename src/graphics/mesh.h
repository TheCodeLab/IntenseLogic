#ifndef ILG_MESH_H
#define ILG_MESH_H

#include "common/base.h"
#include "graphics/bindable.h"

struct ilG_drawable3d;
struct ilA_mesh;
struct ilG_context;

extern il_type ilG_mesh_type;
extern const ilG_bindable ilG_mesh_bindable;

struct ilG_drawable3d* ilG_mesh(const struct ilA_mesh* self, struct ilG_context *context);

struct ilG_drawable3d* ilG_mesh_fromfile(const char *name, struct ilG_context *context);

#endif

