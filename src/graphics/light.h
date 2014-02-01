#ifndef ILG_LIGHT_H
#define ILG_LIGHT_H

#include <GL/glew.h>

#include "math/vector.h"
#include "common/base.h"
#include "common/world.h"

struct ilG_context;

enum ilG_light_type {
    ILG_POINT,
    ILG_DIRECTIONAL
};

typedef struct ilG_light {
    il_positionable positionable;
    il_vec4 color;
    enum ilG_light_type type;
    GLuint texture; // shadow map
    float radius;
} ilG_light;

extern il_type ilG_light_type;

ilG_light *ilG_light_new();
void ilG_light_free(ilG_light *self);

void ilG_light_setPositionable(ilG_light *self, il_positionable pos);
void ilG_light_add(ilG_light *self, struct ilG_context* context);

#endif

