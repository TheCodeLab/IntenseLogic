// Copyright (c) 2012 Code Lab
//
// This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
//
//     2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
//
//     3. This notice may not be removed or altered from any source distribution.
//

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
      il_Common_log(1, str ": %s (%i)\n", ##__VA_ARGS__, il_Graphics_strerror(err), err); \
  }

GLuint il_Graphics_makeShader(GLenum type, il_Common_String source);
void il_Graphics_linkProgram(GLuint program);
void il_Graphics_bindUniforms(GLuint program, const il_Graphics_Camera * camera, const il_Common_Positionable * object);

#endif
