#ifndef IL_GRAPHICS_SHAPE_H
#define IL_GRAPHICS_SHAPE_H

#include "common/positionable.h"
#include "graphics/drawable3d.h"

enum ilG_shapeType {
  ilG_box,
  ilG_cylinder,
  ilG_sphere,
  ilG_plane
};

typedef struct ilG_shape ilG_shape;

ilG_shape * ilG_shape_new(il_positionable * parent, int type);

#endif
