#ifndef IL_GRAPHICS_DRAWABLE3D_H
#define IL_GRAPHICS_DRAWABLE3D_H

//#include <GL/glew.h>

#include "common/positionable.h"
#include "camera.h"

struct ilG_drawable3d;

typedef void (*ilG_drawable3d_cb)(const ilG_camera*,
struct ilG_drawable3d*, const struct timeval*);

typedef struct ilG_drawable3d {
  int type;
  il_positionable* positionable;
  //GLuint shader;
  void *drawcontext;
  ilG_drawable3d_cb draw;
} ilG_drawable3d;

typedef struct ilG_drawable3dIterator ilG_drawable3dIterator;

void ilG_drawable3d_setPositionable(ilG_drawable3d*,
  il_positionable*);

ilG_drawable3d* ilG_drawable3d_iterate(il_positionable*,
  ilG_drawable3dIterator**);

//ilG_drawable3d * ilG_drawable3d_new(il_positionable * parent);

#endif
