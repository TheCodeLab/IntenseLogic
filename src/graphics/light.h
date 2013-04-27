#ifndef ILG_LIGHT_H
#define ILG_LIGHT_H

#include <GL/glew.h>

#include "math/vector.h"
#include "common/base.h"
#include "common/positionable.h"

struct ilG_context;

typedef struct ilG_phong {
    il_vec4 ambient, diffuse, specular;
    float specular_co, transparency;
} ilG_phong;

enum ilG_light_type {
    ILG_POINT,
    ILG_DIRECTIONAL
};

typedef struct ilG_light {
    struct il_positionable positionable;
    il_vec4 color;
    enum ilG_light_type type;
    GLuint texture; // shadow map
    float radius;
} ilG_light;

extern il_type ilG_light_type;

//ilG_light* ilG_light_new();
#define ilG_light_new() il_new(&ilG_light_type)
void ilG_light_add(ilG_light*, struct ilG_context* context);
void ilG_bindPhong(ilG_phong* phong, GLuint program, const char *prefix);
/*void ilG_bindLight(ilG_light* light, GLuint program, const char *prefix);
void ilG_bindLights(struct ilG_context* context, GLuint program, const char *prefix);*/

#endif

