#ifndef IL_GRAPHICS_DRAWABLE3D_H
#define IL_GRAPHICS_DRAWABLE3D_H

//#include <GL/glew.h>

#include "common/positionable.h"
#include "camera.h"

struct il_Graphics_Drawable3d;

typedef void (*il_Graphics_Drawable3d_cb)(const il_Graphics_Camera*, 
struct il_Graphics_Drawable3d*, const struct timeval*);

typedef struct il_Graphics_Drawable3d {
  int type;
  il_Common_Positionable* positionable;
  //GLuint shader;
  void *drawcontext;
  il_Graphics_Drawable3d_cb draw;
} il_Graphics_Drawable3d;

typedef struct il_Graphics_Drawable3dIterator il_Graphics_Drawable3dIterator;

void il_Graphics_Drawable3d_setPositionable(il_Graphics_Drawable3d*, 
  il_Common_Positionable*);

il_Graphics_Drawable3d* il_Graphics_Drawable3d_iterate(il_Common_Positionable*,
  il_Graphics_Drawable3dIterator**);

//il_Graphics_Drawable3d * il_Graphics_Drawable3d_new(il_Common_Positionable * parent);

#endif
