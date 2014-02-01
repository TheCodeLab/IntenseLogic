#include "camera.h"

ilG_camera *ilG_camera_new()
{
    ilG_camera* camera = calloc(1, sizeof(ilG_camera));
    camera->projection_matrix = il_mat_identity(NULL);
    camera->sensitivity = 0.002;
    return camera;
}

void ilG_camera_free(ilG_camera *self)
{
    free(self);
}

void ilG_camera_setPositionable(ilG_camera *self, il_positionable pos)
{
    self->positionable = pos;
}

void ilG_camera_setMatrix(ilG_camera *self, il_mat mat)
{
    self->projection_matrix = mat;
}

void ilG_camera_setMovespeed(ilG_camera* camera, il_vec3 movespeed, float pixels_per_radian)
{
    camera->movespeed = movespeed;
    camera->sensitivity = 1.0/pixels_per_radian;
}
