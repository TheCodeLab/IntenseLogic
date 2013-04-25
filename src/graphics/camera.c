#include "camera.h"

#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glfw.h>

#include "common/positionable.h"

void camera_init(void *self)
{
    ilG_camera* camera = self;
    camera->projection_matrix = il_mat_identity(NULL);
    camera->sensitivity = 0.002;
}

il_type ilG_camera_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = camera_init,
    .name = "il.graphics.camera",
    .registry = NULL,
    .size = sizeof(ilG_camera),
    .parent = &il_positionable_type
};

void ilG_camera_setMovespeed(ilG_camera* camera, il_vec3 movespeed, float pixels_per_radian)
{
    camera->movespeed = movespeed;
    camera->sensitivity = 1.0/pixels_per_radian;
}
