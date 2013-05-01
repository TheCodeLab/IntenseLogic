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
    light->color = il_vec4_new();
}

il_type ilG_light_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = light_init,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.light",
    .registry = NULL,
    .size = sizeof(ilG_light),
    .parent = &il_positionable_type
};

void ilG_light_add(ilG_light* self, ilG_context* context)
{
    IL_APPEND(context->lights, self);
    //context->lightdata.invalidated = 1; // TODO: when uniform caching is setup for lighting stage, fix this
}

void ilG_bindPhong(ilG_phong* phong, GLuint program, const char *prefix)
{
    size_t len;
    char *str = NULL;
    const char *fmt = "%s.%s";
#define BIND(fn, T, loc, ...)                           \
    len = snprintf(NULL, 0, fmt, prefix, __VA_ARGS__);  \
    str = realloc(str, len + 1);                        \
    sprintf(str, fmt, prefix, __VA_ARGS__);             \
    fn(glGetUniformLocation(program, str), 1,           \
        (T)&phong->loc);
    BIND(glUniform3fv,  GLfloat*,   ambient,        "ambient"       );
    BIND(glUniform3fv,  GLfloat*,   diffuse,        "diffuse"       );
    BIND(glUniform3fv,  GLfloat*,   specular,       "specular"      );
    BIND(glUniform1fv,  GLfloat*,   specular_co,    "specular_co"   );
#undef BIND
    free(str);

}

/*void ilG_bindLight(ilG_light* light, GLuint program, const char *prefix)
{
    size_t len;
    char *str = NULL;
    const char *fmt = "%s.%s";
#define BIND(fn, T, loc, ...)                           \
    len = snprintf(NULL, 0, fmt, prefix, __VA_ARGS__);  \
    str = realloc(str, len + 1);                        \
    sprintf(str, fmt, prefix, __VA_ARGS__);             \
    fn(glGetUniformLocation(program, str), 1,           \
        (T)&light->loc);
    BIND(glUniform1iv,  GLint*,     texture,                    "texture"       );
    BIND(glUniform1fv,  GLfloat*,   radius,                     "radius"        );
#undef BIND
    ilG_bindPhong(&light->phong, program, prefix);
    free(str);
}

void ilG_bindLights(ilG_context* context, GLuint program, const char *prefix)
{
    unsigned int i;
    char *str = NULL;
    size_t len;
    const char *fmt = "%s[%i]";
    for (i = 0; i < context->lights.length; i++) {
        len = sprintf(NULL, fmt, prefix, i);
        str = realloc(str, len+1);
        sprintf(str, fmt, prefix, i);
        ilG_bindLight(context->lights.data[i], program, prefix);
    }
    free(str);
}*/

