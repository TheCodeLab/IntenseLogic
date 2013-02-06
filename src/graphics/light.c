#include "light.h"

#include <stdlib.h>
#include <GL/glew.h>
#include <stdio.h>

#include "graphics/context.h"
#include "common/positionable.h"

ilG_light* ilG_light_new()
{
    ilG_light* light = calloc(1, sizeof(ilG_light));
    return light;
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
    BIND(glUniform1fv,  GLfloat*,   transparency,   "transparency"  );
#undef BIND
    free(str);

}

void ilG_bindLight(ilG_light* light, GLuint program, const char *prefix)
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
    BIND(glUniform3fv,  GLfloat*,   positionable->position,     "position"      );
    BIND(glUniform4fv,  GLfloat*,   positionable->rotation,     "rotation"      );
    BIND(glUniform1iv,  GLint*,     type,                       "type"          );
    BIND(glUniform1iv,  GLint*,     texture,                    "texture"       );
    BIND(glUniform1fv,  GLfloat*,   radius,                     "radius"        );
    BIND(glUniform1fv,  GLfloat*,   intensity,                  "intensity"     );
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
}

