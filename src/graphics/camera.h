#include "common/positionable.h"

typedef struct il_Graphics_Camera {
        il_Common_Positionable* positionable;
        unsigned refs;
} il_Graphics_Camera;

il_Graphics_Camera* il_Graphics_Camera_new();

void il_Graphics_Camera_render(il_Graphics_Camera* camera);
void il_Graphics_Camera_translate(il_Graphics_Camera* camera, float x, float y, float z);
