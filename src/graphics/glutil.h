#ifndef IL_GRAPHICS_GLUTIL_H
#define IL_GRAPHICS_GLUTIL_H

#include <GL/glew.h>

#include "common/string.h"
#include "common/log.h"
#include "common/positionable.h"
#include "graphics/camera.h"

const char * ilG_strerror(GLenum err);

void ilG_testError_(const char *file, int line, const char *func,
  const char* fmt, ...);

#define ilG_testError(...) ilG_testError_(__FILE__, __LINE__, \
  __func__, __VA_ARGS__);
#define IL_GRAPHICS_TESTERROR ilG_testError

GLuint ilG_makeShader(GLenum type, il_string source);
void ilG_linkProgram(GLuint program);
void ilG_bindUniforms(GLuint program, const ilG_camera *camera,
  const il_positionable * object);

#endif
