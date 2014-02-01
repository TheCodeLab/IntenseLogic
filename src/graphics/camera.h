#ifndef IL_GRAPHICS_CAMERA_H
#define IL_GRAPHICS_CAMERA_H

#include "math/matrix.h"
#include "math/vector.h"
#include "common/world.h"

typedef struct ilG_camera {
    il_positionable positionable;
    il_vec3 movespeed;
    il_mat projection_matrix;
    float sensitivity;
} ilG_camera;

ilG_camera *ilG_camera_new();
void ilG_camera_free(ilG_camera *self);

void ilG_camera_setPositionable(ilG_camera *self, il_positionable pos);
void ilG_camera_setMatrix(ilG_camera *self, il_mat mat);
void ilG_camera_setMovespeed(ilG_camera* camera, il_vec3 movespeed, float radians_per_pixel);

#endif

