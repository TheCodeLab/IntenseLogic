#ifndef IL_GRAPHICS_SHAPE_H
#define IL_GRAPHICS_SHAPE_H

#include "common/base.h"

struct ilG_drawable3d;

extern struct ilG_drawable3d *ilG_box, 
                             *ilG_cylinder, 
                             *ilG_icosahedron, 
                             *ilG_plane;

il_type ilG_shape_type;

#endif
