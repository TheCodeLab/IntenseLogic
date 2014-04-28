#include "glutil.h"

#include <stdlib.h>
#include <GL/glew.h>
#include <stdarg.h>
#include <stdio.h>

#include "math/matrix.h"
#include "graphics/camera.h"
#include "util/log.h"
#include "util/logger.h"

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
        va_list ap;
        va_start(ap, fmt);
        char msg[4096];
        vsnprintf(msg, 4096, fmt, ap);
        va_end(ap);

        char reason[64];
        snprintf(reason, 64, "%s (%i)", ilG_strerror(err), err);
        
        il_logmsg *lmsg = il_logmsg_new(1);
        il_logmsg_setLevel(lmsg, IL_ERROR);
        il_logmsg_copyMessage(lmsg, msg);
        il_logmsg_copyReason(lmsg, reason);
        il_logmsg_setBtFile(lmsg, 0, file, line, func);
        il_logger *logger = il_logger_stderr; // TODO
        il_logger_log(logger, lmsg);
    }
}

GLuint ilG_makeShader(GLenum type, il_string *source)
{
    if (!source) {
        il_error("Null source");
        return 0;
    }
    IL_GRAPHICS_TESTERROR("Unknown");

    GLuint shader = glCreateShader(type);
    IL_GRAPHICS_TESTERROR("Unable to create shader");

    glShaderSource(shader, 1, (const GLchar**)&source->data, (const GLint*)&source->length);
    IL_GRAPHICS_TESTERROR("Unable to set shader source");

    glCompileShader(shader);
    IL_GRAPHICS_TESTERROR("Unable to compile shader");

    GLint status, len;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 1) {
        char * str = calloc(1, len);
        glGetShaderInfoLog(shader, len, NULL, str);
        if (status == GL_TRUE) {
            il_debug("%s Shader info log: %s", type == GL_VERTEX_SHADER? "Vertex" : "Fragment", str);
        } else {
            il_error("%s Shader info log: %s", type == GL_VERTEX_SHADER? "Vertex" : "Fragment", str);
        }
        free(str);
        if (status == GL_FALSE) {
            return 0;
        }
    }

    return shader;
}

int ilG_linkProgram(GLuint program)
{
    il_log("Linking program");
    glLinkProgram(program);
    IL_GRAPHICS_TESTERROR("Unable to link program");

    GLint status, len;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
    if (len > 1) {
        char * str = calloc(1, len);
        glGetProgramInfoLog(program, len, NULL, str);
        if (status == GL_TRUE) {
            il_debug("Program info log: %s", str);
        } else {
            il_error("Program info log: %s", str);
        }
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
        if (status == GL_TRUE) {
            il_debug("Program info log: %s", str);
        } else {
            il_error("Program info log: %s", str);
        }
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
        mvp = camera->projection_matrix;
    } else {
        mvp = il_mat_identity();
    }
    if (filter & ILG_VIEW_R) {
        il_quat q = il_positionable_getRotation(&camera->positionable);
        il_mat rotate = il_mat_rotate(q);
        mvp = il_mat_mul(mvp, rotate);
    }
    if (filter & ILG_VIEW_T) {
        il_vec4 v = il_vec3_to_vec4(il_positionable_getPosition(&camera->positionable), 1.0);
        v.x = -v.x;
        v.y = -v.y;
        v.z = -v.z;

        il_mat translate = il_mat_translate(v);
        mvp = il_mat_mul(mvp, translate);
    }
    if (filter & ILG_MODEL_T) {
        il_vec4 v = il_vec3_to_vec4(il_positionable_getPosition(object), 1.0);
        il_mat mat = il_mat_translate(v);
        mvp = il_mat_mul(mvp, mat);
    }
    if (filter & ILG_MODEL_R) {
        il_mat mat = il_mat_rotate(il_positionable_getRotation(object));
        mvp = il_mat_mul(mvp, mat);
    }
    if (filter & ILG_MODEL_S) {
        il_mat mat = il_mat_scale(il_vec3_to_vec4(il_positionable_getSize(object), 1.0));
        mvp = il_mat_mul(mvp, mat);
    }
    if (filter & ILG_INVERSE) {
        mvp = il_mat_invert(mvp);
    }
    if (filter & ILG_TRANSPOSE) {
        mvp = il_mat_transpose(mvp);
    }
    
    return mvp;
}

void ilG_bindMVP(GLint location, enum ilG_transform filter, const ilG_camera * camera, const il_positionable * object)
{
    ilG_testError("Unknown");
    il_mat mat = ilG_computeMVP(filter, camera, object);
    /*if (!mat) {
        return;
    }*/
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
    
    glUniformMatrix4fv(location, 1, GL_TRUE, mat.data);
    ilG_testError("glUniformMatrix4fv failed");
}

