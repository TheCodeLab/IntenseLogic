#include <stdlib.h>
#include "GL/gl.h"

#include "common/positionable.h"
#include "camera.h"

il_Graphics_Camera* il_Graphics_Camera_new() {
	il_Graphics_Camera* camera = malloc(sizeof(il_Graphics_Camera));
	il_Common_Positionable* positionable = malloc(sizeof(il_Common_Positionable));
	positionable->position = (sg_Vector3){0, 0, 0};
	camera->positionable = positionable;
	return camera;
}

void il_Graphics_Camera_render(il_Graphics_Camera* camera) {
	glTranslatef(-camera->positionable->position.x, -camera->positionable->position.y, -camera->positionable->position.z);
}

void il_Graphics_Camera_translate(il_Graphics_Camera* camera, float x, float y, float z) {
	camera->positionable->position.x = camera->positionable->position.x + x;
	camera->positionable->position.y = camera->positionable->position.y + y;
	camera->positionable->position.z = camera->positionable->position.z + z;
}
