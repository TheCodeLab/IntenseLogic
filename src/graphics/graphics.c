#include "graphics.h"

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <sys/time.h>

#include "graphics/camera.h"
#include "common/event.h"
#include "common/input.h"
#include "common/matrix.h"
#include "common/base.h"
#include "common/keymap.h"
#include "graphics/context.h"
#include "common/log.h"
#include "graphics/shape.h"
#include "asset/asset.h"
#include "common/world.h"
#include "graphics/drawable3d.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "graphics/tracker.h"

extern unsigned time(unsigned*);

int width = 800;
int height = 600;
il_world* world;
il_keymap * keymap;
ilG_shape* shape;
ilG_context* context;

static void draw();
static void quit();
static GLvoid error_cb(GLenum source, GLenum type, GLuint id, GLenum severity,
                       GLsizei length, const GLchar* message, GLvoid* userParam);

static void GLFWCALL key_cb(int key, int action)
{
    il_log(4, "Key %c", key);
    if (action == GLFW_PRESS)
        ilE_pushnew(IL_INPUT_KEYDOWN, sizeof(int), &key);
    else
        ilE_pushnew(IL_INPUT_KEYUP, sizeof(int), &key);
}

static void GLFWCALL mouse_cb(int button, int action)
{
    //il_log(4, "Mouse %i", button);
    if (action == GLFW_PRESS)
        ilE_pushnew(IL_INPUT_MOUSEDOWN, sizeof(int), &button);
    else
        ilE_pushnew(IL_INPUT_MOUSEUP, sizeof(int), &button);
}

static void GLFWCALL mousemove_cb(int x, int y)
{
    ilI_mouseMove mousemove =
    (ilI_mouseMove) {
        x, y
    };
    ilE_pushnew(IL_INPUT_MOUSEMOVE, sizeof(ilI_mouseMove), &mousemove);
}

static void GLFWCALL mousewheel_cb(int pos)
{
    ilI_mouseWheel mousewheel =
    (ilI_mouseWheel) {
        0, pos
    };
    ilE_pushnew(IL_INPUT_MOUSEWHEEL, sizeof(ilI_mouseWheel), &mousewheel);
}

static void context_setup()
{
    if (!glfwInit()) {
        il_log(0, "glfwInit() failed");
        abort();
    }

    {
        int major, minor, rev;
        glfwGetVersion(&major, &minor, &rev);
        il_log(3, "Using GLFW version %i.%i.%i", major, minor, rev);
    }

    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
#ifdef __APPLE__
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
#else
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);
#endif
#ifdef DEBUG
    glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    if (!glfwOpenWindow(width, height, 8, 8, 8, 8, 32, 8, GLFW_WINDOW)) {
        il_log(0, "glfwOpenWindow() failed");
        abort();
    }

    // register glfw stuff
    glfwSetCharCallback(&key_cb);
    glfwSetMouseButtonCallback(&mouse_cb);
    glfwSetMousePosCallback(&mousemove_cb);
    glfwSetMouseWheelCallback(&mousewheel_cb);

    // GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        il_log(0, "glewInit() failed: %s", glewGetErrorString(err));
        abort();
    }
    il_log(3, "Using GLEW %s", glewGetString(GLEW_VERSION));

#ifndef __APPLE__
    if (!GLEW_VERSION_3_1) {
        il_log(1, "GL version 3.1 is required, your Segfault Insurance is now invalid");
    }
#endif

    if (GLEW_ARB_debug_output) {
        glDebugMessageCallbackARB((GLDEBUGPROCARB)&error_cb, NULL);
        il_log(3, "ARB_debug_output present, enabling advanced errors");
    } else
        il_log(3, "ARB_debug_output missing");

    glfwSwapInterval(1); // 1:1 ratio of frames to vsyncs
}

static void scene_setup()
{
    // create the world
    world = il_world_new();
    context = calloc(sizeof(ilG_context), 1);
    context->world = world;
    world->context = context;
    context->camera = ilG_camera_new(il_positionable_new(world));
    context->camera->movespeed = (il_Vector3) {1,1,1};
    context->camera->projection_matrix = il_Matrix_perspective(
        75, (float)width/(float)height, 0.25, 100);
    ilG_camera_setEgoCamKeyHandlers(context->camera, keymap);

    il_positionable * shape_positionable = il_positionable_new(world);
    shape_positionable->position = (il_Vector3) {1,0,0};
    shape = ilG_shape_new(shape_positionable,ilG_box);
    if (!shape) {
        il_log(0, "Failed to create demo shape");
        abort();
    }
    ilG_drawable3d_assignId((ilG_drawable3d*)shape);

    /*il_terrain *ter = il_terrain_new();
      il_terrain_heightmapFromSeed(
      ter,
      0xD34DB33F, // seed, does nothing atm
      1.0, // resolution
      100 //viewdistance
      );
      ilG_terrain_new(ter, il_positionable_new(world));
      */

}

static void event_setup()
{
    ilE_register(IL_GRAPHICS_TICK, (ilE_callback)&draw, NULL);
    ilE_register(IL_BASE_SHUTDOWN, (ilE_callback)&quit, NULL);
    int hz = glfwGetWindowParam(GLFW_REFRESH_RATE);
    struct timeval *tv = calloc(1, sizeof(struct timeval));
    tv->tv_usec = hz>0? 1000000.0/hz : 1000000.0/60;
    ilE_timer(ilE_new(IL_GRAPHICS_TICK, 0, NULL), tv); // kick off the draw loop
}

void ilG_init()
{
    keymap = calloc(1, sizeof(il_keymap));
    il_keymap_defaults(keymap);
    il_keymap_parse("keymap.ini", keymap);
    // setup our shader directory
    ilA_registerReadDir(il_fromC("shaders"),0);

    // Setup GL context (glfw, glew, etc.)
    context_setup();
    
    // GL setup
    glClearColor(0,0,0,1);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    
    // Setup the scene stuff (camera, world, etc.)
    scene_setup();
    
    // register events
    event_setup();
}

static void draw()
{
    if (!glfwGetWindowParam(GLFW_OPENED)) {
        ilE_pushnew(IL_BASE_SHUTDOWN, 0, NULL);
        return;
    }
    //il_log(5, "Rendering frame");
    struct timeval tv;
    il_positionable* pos;
    ilG_drawable3d* drawable;
    ilG_material* material;
    ilG_texture* texture;
    ilG_trackiterator * iter = ilG_trackiterator_new(context);

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gettimeofday(&tv, NULL);

    while (ilG_trackIterate(iter)) {
        if (drawable != ilG_trackGetDrawable(iter)) {
            drawable->unbind(drawable, drawable->unbind_ctx);
            drawable = ilG_trackGetDrawable(iter);
            drawable->bind(drawable, drawable->bind_ctx);
        }
        if (material != ilG_trackGetMaterial(iter)) {
            material->unbind(material, material->unbind_ctx);
            material = ilG_trackGetMaterial(iter);
            material->bind(material, material->bind_ctx);
        }
        if (texture != ilG_trackGetTexture(iter)) {
            texture->unbind(texture, texture->unbind_ctx);
            texture = ilG_trackGetTexture(iter);
            texture->bind(texture, texture->bind_ctx);
        }
        drawable->update(drawable, material, texture, drawable->update_ctx);
        material->update(material, drawable, texture, material->update_ctx);
        texture->update(texture, drawable, material, texture->update_ctx);

        pos = ilG_trackGetPositionable(iter);

        drawable->draw(context->camera, drawable, &tv, pos);
    }

    glfwSwapBuffers();
}

static GLvoid error_cb(GLenum source, GLenum type, GLuint id, GLenum severity,
                       GLsizei length, const GLchar* message, GLvoid* user)
{
    (void)id, (void)severity, (void)length, (void)user;
    const char *ssource;
    switch(source) {
        case GL_DEBUG_SOURCE_API_ARB:               ssource="API";              break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:     ssource="Window System";    break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:   ssource="Shader Compiler";  break;
        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:       ssource="Third Party";      break;
        case GL_DEBUG_SOURCE_APPLICATION_ARB:       ssource="Application";      break;
        case GL_DEBUG_SOURCE_OTHER_ARB:             ssource="Other";            break;
        default: ssource="???";
    }
    const char *stype;
    switch(type) {
        case GL_DEBUG_TYPE_ERROR_ARB:               stype="Error";                  break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: stype="Deprecated Behaviour";   break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:  stype="Undefined Behaviour";    break;
        case GL_DEBUG_TYPE_PORTABILITY_ARB:         stype="Portability";            break;
        case GL_DEBUG_TYPE_PERFORMANCE_ARB:         stype="Performance";            break;
        case GL_DEBUG_TYPE_OTHER_ARB:               stype="Other";                  break;
        default: stype="???";
    }
    const char *sseverity;
    switch(severity) {
        case GL_DEBUG_SEVERITY_HIGH_ARB:    sseverity="HIGH";   break;
        case GL_DEBUG_SEVERITY_MEDIUM_ARB:  sseverity="MEDIUM"; break;
        case GL_DEBUG_SEVERITY_LOW_ARB:     sseverity="LOW";    break;
        default: sseverity="???";
    }
    fprintf(il_logfile, "OpenGL %s (%s) %s: %s\n", ssource, stype,
            sseverity, message);
}

static void quit()
{
    glfwTerminate();
}

