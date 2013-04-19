#include "camera.h"

#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glfw.h>

#include "common/positionable.h"

ilG_camera* ilG_camera_new(il_positionable * parent)
{
    ilG_camera* camera = calloc(1, sizeof(ilG_camera));
    camera->positionable = parent;
    camera->projection_matrix = il_mat_identity(NULL);
    camera->sensitivity = 0.002;
    camera->refs = 1;
    return camera;
}

void ilG_camera_setMovespeed(ilG_camera* camera, il_vec3 movespeed, float pixels_per_radian)
{
    camera->movespeed = movespeed;
    camera->sensitivity = 1.0/pixels_per_radian;
}
