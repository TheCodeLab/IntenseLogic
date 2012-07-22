#ifndef IL_GRAPHICS_DRAWABLE3D_H
#define IL_GRAPHICS_DRAWABLE3D_H

#include <GL/gl.h>

#include "common/positionable.h"

enum il_Graphics_Shapes {
  Box,
  Cylinder,
  Sphere,
  Plane
}

typedef struct il_Graphics_Drawable3d {
  il_Common_Positionable* positionable;
  GLuint shader;
  GLuint texture;
  void (*draw)(struct il_Graphics_Drawable3d*);
  unsigned refs;
} il_Graphics_Drawable3d;

#endif
