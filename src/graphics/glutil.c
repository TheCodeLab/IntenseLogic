#include "glutil.h"

#include <stdlib.h>
#include <GL/glew.h>

#include "common/matrix.h"
#include "graphics/camera.h"
#include "common/positionable.h"

const char * il_Graphics_strerror(GLenum err) {
  char * res;
  switch(err) {
    case GL_NO_ERROR:           res = "GL_NO_ERROR"; break;
    case GL_INVALID_ENUM:       res = "GL_INVALID_ENUM"; break;
    case GL_INVALID_VALUE:      res = "GL_INVALID_VALUE"; break;
    case GL_INVALID_OPERATION:  res = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
    case GL_OUT_OF_MEMORY:      res = "GL_OUT_OF_MEMORY"; break;
    case GL_STACK_OVERFLOW:     res = "GL_STACK_OVERFLOW"; break;
    case GL_STACK_UNDERFLOW:    res = "GL_STACK_UNDERFLOW"; break;
    default:                    res = "UNKNOWN ERROR"; break;
  }
  return res;
}

GLuint il_Graphics_makeShader(GLenum type, il_Common_String source) {
  GLuint shader = glCreateShader(type);
  IL_GRAPHICS_TESTERROR("Unable to create shader");

  glShaderSource(shader, 1, (const GLchar**)&source.data, (GLint*)&source.length);
  IL_GRAPHICS_TESTERROR("Unable to set shader source");

  glCompileShader(shader);
  IL_GRAPHICS_TESTERROR("Unable to compile shader");

  GLint status, len;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
  if (len > 1) {
    char * str = calloc(1, len);
    glGetShaderInfoLog(shader, len, NULL, str);
    il_Common_log(status == GL_TRUE? 4 : 1, 
    "Shader info log: \n"
    "---- BEGIN SHADER INFO LOG ----\n"
    "%s\n"
    "---- END SHADER INFO LOG ----\n", str);
    free(str);
  }

  return shader;
}

void il_Graphics_linkProgram(GLuint program) {
  glLinkProgram(program);
  IL_GRAPHICS_TESTERROR("Unable to link program");
  
  GLint status, len;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
  if (len > 1) {
    char * str = calloc(1, len);
    glGetProgramInfoLog(program, len, NULL, str);
    il_Common_log(status == GL_TRUE? 4 : 1, "Program info log: \n"
    "---- BEGIN PROGRAM INFO LOG ----\n"
    "%s\n"
    "---- END PROGRAM INFO LOG ----\n", str);
    free(str);
  }
  
  glValidateProgram(program);
  IL_GRAPHICS_TESTERROR("Unable to validate program");
  
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
  if (len > 1) {
    char * str = calloc(1, len);
    glGetProgramInfoLog(program, len, NULL, str);
    il_Common_log(status == GL_TRUE? 4 : 1, "Program info log: \n"
    "---- BEGIN PROGRAM INFO LOG ----\n"
    "%s\n"
    "---- END PROGRAM INFO LOG ----\n", str);
    free(str);
  }
}

void il_Graphics_bindUniforms(GLuint program, const il_Graphics_Camera * camera, const il_Common_Positionable * object) {
  GLint utransform;
  utransform = glGetUniformLocation(program, "transform");
  
  sg_Matrix cam = sg_Matrix_mul(
    sg_Matrix_translate(camera->positionable->position),
    sg_Matrix_rotate_q(camera->positionable->rotation)
  );
  
  sg_Matrix view;
  
  int res = sg_Matrix_invert(cam, &view);
  if (res!=0)
    il_Common_log(2, "Couldn't invert view matrix?");
  
  sg_Matrix model = sg_Matrix_mul(
    sg_Matrix_rotate_q(object->rotation),
    sg_Matrix_mul(
    sg_Matrix_scale(object->size),
    sg_Matrix_translate(object->position)
  ));
  
  sg_Matrix mat = sg_Matrix_mul(
    camera->projection_matrix,
    sg_Matrix_mul(
    view,
    model
  ));
  
  glUniformMatrix4fv(utransform, 1, GL_TRUE, (const GLfloat*)&mat.data);
}
