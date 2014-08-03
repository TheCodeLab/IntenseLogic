#ifndef IL_GRAPHICS_GLUTIL_H
#define IL_GRAPHICS_GLUTIL_H

#include <GL/glew.h>

#include "util/ilstring.h"
#include "common/world.h"
#include "math/matrix.h"

const char * ilG_strerror(GLenum err);

void ilG_testError_(const char *file, int line, const char *func,
  const char* fmt, ...);

#ifdef ILG_ENABLE_GETERROR
#define ilG_testError(...) ilG_testError_(__FILE__, __LINE__, \
  __func__, __VA_ARGS__);
#else
#define ilG_testError(...)
#endif
#define IL_GRAPHICS_TESTERROR ilG_testError

GLuint ilG_makeShader(GLenum type, il_string *source);
int /*failure*/ ilG_linkProgram(GLuint program);

enum ilG_transform {
    ILG_PROJECTION  = 0x1,
    ILG_VIEW_R      = 0x2,
    ILG_VIEW_T      = 0x4,
    ILG_MODEL_R     = 0x8,
    ILG_MODEL_T     = 0x10,
    ILG_MODEL_S     = 0x20,
    ILG_INVERSE     = 0x80,
    ILG_TRANSPOSE   = 0x40,
    ILG_VIEW = ILG_VIEW_R | ILG_VIEW_T,
    ILG_VP = ILG_PROJECTION | ILG_VIEW,
    ILG_MODEL = ILG_MODEL_R | ILG_MODEL_T | ILG_MODEL_S,
    ILG_MVP = ILG_VP | ILG_MODEL,
    ILG_IMT = ILG_MODEL | ILG_VIEW_T | ILG_TRANSPOSE | ILG_INVERSE,
};
/*il_mat ilG_computeMVP(enum ilG_transform filter, const ilG_camera *camera,
    const il_positionable * object);
void ilG_bindMVP(GLint location, enum ilG_transform filter,
const ilG_camera *camera, const il_positionable * object);*/

#endif
