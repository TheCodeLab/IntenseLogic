#include "glutil.h"

#include <stdlib.h>
#include <GL/glew.h>
#include <stdarg.h>

#include "math/matrix.h"
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
    IL_GRAPHICS_TESTERROR("Unknown");

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
                      "%s Shader info log: \n"
                      "---- BEGIN SHADER INFO LOG ----\n"
                      "%s\n"
                      "---- END SHADER INFO LOG ----\n", type == GL_VERTEX_SHADER? "Vertex" : "Fragment", str);
        free(str);
        if (status == GL_FALSE) {
            return 0;
        }
    }

    return shader;
}

int ilG_linkProgram(GLuint program)
{
    il_log(3, "Linking program");
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
        if (status == GL_FALSE) {
            return 1;
        }
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
        if (status == GL_FALSE) {
            return 1;
        }
    }
    return 0;
}

il_mat ilG_computeMVP(enum ilG_transform filter, const ilG_camera* camera, const il_positionable* object)
{
    il_mat mvp;
    if (filter & ILG_PROJECTION) {
        mvp = il_mat_copy(camera->projection_matrix);
    } else {
        mvp = il_mat_identity(NULL);
    }
    if (filter & ILG_VIEW) {
        il_vec4 v = camera->positionable->position;

        il_quat q = il_quat_copy(camera->positionable->rotation);
        q[0] = -q[0];
        q[1] = -q[1];
        q[2] = -q[2];
        q[3] = -q[3];

        il_mat rotate = il_mat_rotate(q, NULL);
        il_mat translate = il_mat_translate(v, NULL);
        il_mat view = il_mat_mul(rotate, translate, NULL);
        mvp = il_mat_mul(mvp, view, mvp);
        il_mat_free(rotate);
        il_mat_free(translate);
        il_mat_free(view);
        il_quat_free(q);
    }
    if (filter & ILG_MODEL) {
        il_mat model = il_mat_new();
        il_mat mat1 = il_mat_scale(object->size, NULL);
        il_mat mat2 = il_mat_translate(object->position, NULL);
        model = il_mat_mul(mat1, mat2, model);
        mat1 = il_mat_rotate(object->rotation, mat1);
        il_mat_free(mat2);
        model = il_mat_mul(mat1, model, model);
        mvp = il_mat_mul(mvp, model, mvp);
        il_mat_free(model);
        il_mat_free(mat1);
    }
    if (filter & ILG_INVERSE) {
        mvp = il_mat_invert(mvp, mvp);
    }
    
    return mvp;
}

void ilG_bindMVP(GLint location, enum ilG_transform filter, const ilG_camera * camera, const il_positionable * object)
{
    ilG_testError("Unknown");
    il_mat mat = ilG_computeMVP(filter, camera, object);
    // I really resorted to this to try tracking down a bug with rendering
    /*char *camera_pos = il_vec4_print(camera->positionable->position, NULL, 0),
         *camera_rot = il_quat_print(camera->positionable->rotation, NULL, 0),
         *object_pos = NULL, *object_rot = NULL;
    if (object) {
        object_pos = il_vec4_print(object->position, NULL, 0);
        object_rot = il_quat_print(object->rotation, NULL, 0);
    }
    printf("T<%i> camera<%p, %s, %s> object<%p, %s, %s>", filter, camera, camera_pos, camera_rot, object, object_pos, object_rot);
    free(camera_pos);
    free(camera_rot);
    free(object_pos);
    free(object_rot);
    int i;
    for (i = 0; i < 16; i++) {
        printf(" % .6f", mat[i]);
    }
    printf("\n");*/
    
    glUniformMatrix4fv(location, 1, GL_TRUE, mat);
    ilG_testError("glUniformMatrix4fv failed");
    il_mat_free(mat);
}

