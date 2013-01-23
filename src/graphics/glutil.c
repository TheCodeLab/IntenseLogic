#include "glutil.h"

#include <stdlib.h>
#include <GL/glew.h>
#include <stdarg.h>

#include "common/matrix.h"
#include "graphics/camera.h"
#include "common/positionable.h"

const char * ilG_strerror(GLenum err)
{
    char * res;
    switch(err) {
    case GL_NO_ERROR:
        res = "GL_NO_ERROR";
        break;
    case GL_INVALID_ENUM:
        res = "GL_INVALID_ENUM";
        break;
    case GL_INVALID_VALUE:
        res = "GL_INVALID_VALUE";
        break;
    case GL_INVALID_OPERATION:
        res = "GL_INVALID_OPERATION";
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        res = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;
    case GL_OUT_OF_MEMORY:
        res = "GL_OUT_OF_MEMORY";
        break;
    case GL_STACK_OVERFLOW:
        res = "GL_STACK_OVERFLOW";
        break;
    case GL_STACK_UNDERFLOW:
        res = "GL_STACK_UNDERFLOW";
        break;
    default:
        res = "UNKNOWN ERROR";
        break;
    }
    return res;
}

void ilG_testError_(const char *file, int line, const char *func,
                            const char* fmt, ...)
{
    GLenum err;
    if ((err = glGetError()) != GL_NO_ERROR) {
        fprintf(il_logfile, "%s:%i (%s) %s: ",
                il_prettifyFile(file),
                line,
                func,
                il_loglevel_tostring(1)
               );

        va_list ap;
        va_start(ap, fmt);
        vfprintf(il_logfile, fmt, ap);
        va_end(ap);

        fprintf(il_logfile, ": %s (%i)",
                ilG_strerror(err),
                err
               );

        fputc('\n', il_logfile);
    }
}

GLuint ilG_makeShader(GLenum type, il_string source)
{
    IL_GRAPHICS_TESTERROR("Unknown error before function");

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
        il_log(status == GL_TRUE? 4 : 1,
                      "Shader info log: \n"
                      "---- BEGIN SHADER INFO LOG ----\n"
                      "%s\n"
                      "---- END SHADER INFO LOG ----\n", str);
        free(str);
    }

    return shader;
}

void ilG_linkProgram(GLuint program)
{
    glLinkProgram(program);
    IL_GRAPHICS_TESTERROR("Unable to link program");

    GLint status, len;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
    if (len > 1) {
        char * str = calloc(1, len);
        glGetProgramInfoLog(program, len, NULL, str);
        il_log(status == GL_TRUE? 4 : 1, "Program info log: \n"
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
        il_log(status == GL_TRUE? 4 : 1, "Program info log: \n"
                      "---- BEGIN PROGRAM INFO LOG ----\n"
                      "%s\n"
                      "---- END PROGRAM INFO LOG ----\n", str);
        free(str);
    }
}

void ilG_bindMVP(const char *name, GLuint program, const ilG_camera * camera, const il_positionable * object)
{
    GLint utransform;
    utransform = glGetUniformLocation(program, name);
    ilG_testError("glGetUniformLocation failed");

    il_Vector3 v = camera->positionable->position;
    /*v.x = -v.x;
    v.y = -v.y;
    v.z = -v.z;*/

    il_Quaternion q = camera->positionable->rotation;
    q.x = -q.x;
    q.y = -q.y;
    q.z = -q.z;
    q.w = -q.w;

    il_Matrix view = il_Matrix_mul(
        il_Matrix_rotate_q(q),
        il_Matrix_translate(v)
    );

    //il_Matrix view;

    /*int res = il_Matrix_invert(cam, &view);
    if (res!=0)
        il_log(2, "Couldn't invert view matrix?");*/

    il_Matrix model = il_Matrix_mul(
                          il_Matrix_rotate_q(object->rotation),
                          il_Matrix_mul(
                              il_Matrix_scale(object->size),
                              il_Matrix_translate(object->position)
                          ));

    il_Matrix mat = il_Matrix_mul(
                        camera->projection_matrix,
                        il_Matrix_mul(
                            view,
                            model
                        ));

    glUniformMatrix4fv(utransform, 1, GL_TRUE, (const GLfloat*)&mat.data);
    ilG_testError("glUniformMatrix4fv failed");
}

