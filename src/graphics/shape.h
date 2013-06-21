#ifndef IL_GRAPHICS_SHAPE_H
#define IL_GRAPHICS_SHAPE_H

#include "common/base.h"

struct ilG_drawable3d;
struct ilG_context;

struct ilG_drawable3d* ilG_box(struct ilG_context *context);
struct ilG_drawable3d* ilG_icosahedron(struct ilG_context *context);

extern il_type ilG_shape_type;

#endif
