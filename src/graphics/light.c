#include "light.h"

#include <stdlib.h>
#include <GL/glew.h>
#include <stdio.h>
#include <string.h>

#include "util/array.h"
#include "graphics/context.h"
#include "common/positionable.h"

void light_init(void *obj)
{
    ilG_light *light = obj;
    light->color = il_vec4_new(0, 0, 0, 0);
}

il_type ilG_light_type = {
    .typeclasses = NULL,
    .storage = {NULL},
    .constructor = light_init,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.light",
    .size = sizeof(ilG_light),
    .parent = &il_positionable_type
};

void ilG_light_add(ilG_light* self, ilG_context* context)
{
    IL_APPEND(context->lights, self);
    //context->lightdata.invalidated = 1; // TODO: when uniform caching is setup for lighting stage, fix this
}

