#ifndef IL_GRAPHICS_GLUTIL_H
#define IL_GRAPHICS_GLUTIL_H

#include <GL/glew.h>

#include "common/string.h"
#include "common/log.h"
#include "common/positionable.h"
#include "graphics/camera.h"

const char * il_Graphics_strerror(GLenum err);

void il_Graphics_testError_(const char *file, int line, const char *func, 
  const char* fmt, ...);

#define il_Graphics_testError(...) il_Graphics_testError_(__FILE__, __LINE__, \
  __func__, __VA_ARGS__);
#define IL_GRAPHICS_TESTERROR il_Graphics_testError

GLuint il_Graphics_makeShader(GLenum type, il_Common_String source);
void il_Graphics_linkProgram(GLuint program);
void il_Graphics_bindUniforms(GLuint program, const il_Graphics_Camera *camera,
  const il_Common_Positionable * object);

#endif
