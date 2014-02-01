#include "light.h"

#include <stdlib.h>
#include <GL/glew.h>
#include <stdio.h>
#include <string.h>

#include "util/array.h"
#include "graphics/context.h"

ilG_light *ilG_light_new()
{
    ilG_light *light = calloc(1, sizeof(ilG_light));
    return light;
}

void ilG_light_free(ilG_light *self)
{
    free(self);
}

void ilG_light_setPositionable(ilG_light *self, il_positionable pos)
{
    self->positionable = pos;
}

void ilG_light_add(ilG_light* self, ilG_context* context)
{
    IL_APPEND(context->lights, self);
    //context->lightdata.invalidated = 1; // TODO: when uniform caching is setup for lighting stage, fix this
}

