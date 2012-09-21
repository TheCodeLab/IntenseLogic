#ifndef IL_GRAPHICS_SHAPE_H
#define IL_GRAPHICS_SHAPE_H

#include "common/positionable.h"
#include "graphics/drawable3d.h"

enum il_Graphics_ShapeType {
  il_Graphics_Box,
  il_Graphics_Cylinder,
  il_Graphics_Sphere,
  il_Graphics_Plane
};

typedef struct il_Graphics_Shape il_Graphics_Shape;

il_Graphics_Shape * il_Graphics_Shape_new(il_Common_Positionable * parent, int type);

#endif
