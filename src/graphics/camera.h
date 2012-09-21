#ifndef IL_GRAPHICS_CAMERA_H
#define IL_GRAPHICS_CAMERA_H

#include "common/positionable.h"
#include "common/matrix.h"
#include "common/keymap.h"

typedef struct il_Graphics_Camera {
  il_Common_Positionable* positionable;
  sg_Vector3 movespeed;
  sg_Matrix projection_matrix;
  float sensitivity;
  unsigned refs;
} il_Graphics_Camera;

il_Graphics_Camera* il_Graphics_Camera_new(il_Common_Positionable * parent);

void il_Graphics_Camera_setEgoCamKeyHandlers(il_Graphics_Camera* camera, il_Common_Keymap * keymap);
void il_Graphics_Camera_render(il_Graphics_Camera* camera);
void il_Graphics_Camera_setMovespeed(il_Graphics_Camera* camera, sg_Vector3 movespeed, float radians_per_pixel);
void il_Graphics_Camera_translate(il_Graphics_Camera* camera, sg_Vector3 vec);

#endif
