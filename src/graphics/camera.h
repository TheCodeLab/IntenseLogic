#ifndef IL_GRAPHICS_CAMERA_H
#define IL_GRAPHICS_CAMERA_H

#include "common/positionable.h"
#include "common/matrix.h"
#include "common/vector.h"
#include "common/quaternion.h"
#include "common/keymap.h"

typedef struct ilG_camera {
  il_positionable* positionable;
  il_Vector3 movespeed;
  il_Matrix projection_matrix;
  float sensitivity;
  unsigned refs;
} ilG_camera;

ilG_camera* ilG_camera_new(il_positionable * parent);

void ilG_camera_setEgoCamKeyHandlers(ilG_camera* camera, il_keymap * keymap);
void ilG_camera_setMovespeed(ilG_camera* camera, il_Vector3 movespeed, float radians_per_pixel);

#endif
