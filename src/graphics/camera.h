#ifndef IL_GRAPHICS_CAMERA_H
#define IL_GRAPHICS_CAMERA_H

#include "math/matrix.h"
#include "math/vector.h"
#include "common/positionable.h"

typedef struct ilG_camera {
  il_positionable positionable;
  il_vec3 movespeed;
  il_mat projection_matrix;
  float sensitivity;
} ilG_camera;

extern il_type ilG_camera_type;

#define ilG_camera_new() (ilG_camera*)il_new(&ilG_camera_type) //ilG_camera* ilG_camera_new(struct il_positionable * parent);

void ilG_camera_setMovespeed(ilG_camera* camera, il_vec3 movespeed, float radians_per_pixel);

#endif

