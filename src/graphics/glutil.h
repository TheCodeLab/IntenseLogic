#ifndef IL_GRAPHICS_GLUTIL_H
#define IL_GRAPHICS_GLUTIL_H

#include <GL/glew.h>

#include "common/string.h"
#include "common/log.h"
#include "common/positionable.h"
#include "graphics/camera.h"
#include "math/matrix.h"

const char * ilG_strerror(GLenum err);

void ilG_testError_(const char *file, int line, const char *func,
  const char* fmt, ...);

#define ilG_testError(...) ilG_testError_(__FILE__, __LINE__, \
  __func__, __VA_ARGS__);
#define IL_GRAPHICS_TESTERROR ilG_testError

GLuint ilG_makeShader(GLenum type, il_string source);
int /*failure*/ ilG_linkProgram(GLuint program);

enum ilG_transform {
    ILG_PROJECTION = 1,
    ILG_VIEW = 2,
    ILG_MODEL = 4,
    ILG_INVERSE = 8,
    ILG_VP = ILG_PROJECTION | ILG_VIEW,
    ILG_MVP = ILG_VP | ILG_MODEL,
};
il_mat ilG_computeMVP(enum ilG_transform filter, const ilG_camera *camera, 
    const il_positionable * object);
void ilG_bindMVP(GLint location, enum ilG_transform filter, 
    const ilG_camera *camera, const il_positionable * object);

#endif
