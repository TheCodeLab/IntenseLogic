#ifndef IL_GRAPHICS_GLUTIL_H
#define IL_GRAPHICS_GLUTIL_H

#include <GL/glew.h>

#include "common/string.h"
#include "common/log.h"
#include "common/positionable.h"
#include "graphics/camera.h"

const char * il_Graphics_strerror(GLenum err);

#define IL_GRAPHICS_TESTERROR(str, ...)             \
  {                                                 \
    GLenum err;                                     \
    if ((err = glGetError()) != GL_NO_ERROR)        \
      il_Common_log(1, str ": %s (%i)", ##__VA_ARGS__, il_Graphics_strerror(err), err); \
  }

GLuint il_Graphics_makeShader(GLenum type, il_Common_String source);
void il_Graphics_linkProgram(GLuint program);
void il_Graphics_bindUniforms(GLuint program, const il_Graphics_Camera * camera, const il_Common_Positionable * object);

#endif
