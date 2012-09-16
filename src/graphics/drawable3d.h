#ifndef IL_GRAPHICS_DRAWABLE3D_H
#define IL_GRAPHICS_DRAWABLE3D_H

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "common/positionable.h"
#include "camera.h"

enum il_Graphics_Shapes {
  il_Graphics_Box,
  il_Graphics_Cylinder,
  il_Graphics_Sphere,
  il_Graphics_Plane
};

struct il_Graphics_Drawable3d;

typedef void (*il_Graphics_Drawable3d_cb)(const il_Graphics_Camera*, 
struct il_Graphics_Drawable3d*, const struct timeval*);

typedef struct il_Graphics_Drawable3d {
  il_Common_Positionable* positionable;
  GLuint shader;
  void *drawcontext;
  il_Graphics_Drawable3d_cb draw;
  unsigned refs;
} il_Graphics_Drawable3d;

il_Graphics_Drawable3d * il_Graphics_Drawable3d_new(il_Common_Positionable * parent);

#endif
