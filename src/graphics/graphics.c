#include "graphics.h"

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <GL/glew.h>
#include <GL/glfw.h>

#include "graphics/camera.h"
#include "common/event.h"
#include "common/input.h"
#include "common/matrix.h"
#include "common/base.h"
//#include "common/keymap.h"
#include "graphics/context.h"
#include "common/log.h"
#include "graphics/shape.h"
#include "asset/asset.h"
#include "common/world.h"
#include "graphics/drawable3d.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "graphics/tracker.h"
#include "graphics/glutil.h"
#include "common/string.h"
#include "graphics/arrayattrib.h"
#include "graphics/textureunit.h"

static int width = 800;
static int height = 600;
//static il_world* world;
//static il_keymap * keymap;
static ilG_context* context = NULL;

void ilG_context_setActive(ilG_context* self)
{
    context = self;
}

static void global_draw();
static void draw();
static void quit();
static GLvoid error_cb(GLenum source, GLenum type, GLuint id, GLenum severity,
                       GLsizei length, const GLchar* message, GLvoid* userParam);
void ilG_material_init();
void ilG_shape_init();
void ilG_texture_init();

static void GLFWCALL key_cb(int key, int action)
{
    il_log(4, "Key %c", key);
    if (action == GLFW_PRESS)
        ilE_pushnew(il_queue, IL_INPUT_KEYDOWN, sizeof(int), &key);
    else
        ilE_pushnew(il_queue, IL_INPUT_KEYUP, sizeof(int), &key);
}

static void GLFWCALL mouse_cb(int button, int action)
{
    il_log(4, "Mouse %i", button);
    if (action == GLFW_PRESS)
        ilE_pushnew(il_queue, IL_INPUT_MOUSEDOWN, sizeof(int), &button);
    else
        ilE_pushnew(il_queue, IL_INPUT_MOUSEUP, sizeof(int), &button);
}

static void GLFWCALL mousemove_cb(int x, int y)
{
    static int last_x = 0, last_y = 0;
    ilI_mouseMove mousemove =
    (ilI_mouseMove) {
        x - last_x, y - last_y
    };
    last_x = x;
    last_y = y;
    ilE_pushnew(il_queue, IL_INPUT_MOUSEMOVE, sizeof(ilI_mouseMove), &mousemove);
}

static void GLFWCALL mousewheel_cb(int pos)
{
    ilI_mouseWheel mousewheel =
    (ilI_mouseWheel) {
        0, pos
    };
    ilE_pushnew(il_queue, IL_INPUT_MOUSEWHEEL, sizeof(ilI_mouseWheel), &mousewheel);
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

    IL_GRAPHICS_TESTERROR("Unknown");

    // GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        il_log(0, "glewInit() failed: %s", glewGetErrorString(err));
        abort();
    }
    il_log(3, "Using GLEW %s", glewGetString(GLEW_VERSION));

    IL_GRAPHICS_TESTERROR("glewInit()");

#ifndef __APPLE__
    if (!GLEW_VERSION_3_1) {
        il_log(1, "GL version 3.1 is required, your Segfault Insurance is now invalid");
    }
#endif

    IL_GRAPHICS_TESTERROR("Unknown");
#ifdef DEBUG
    if (GLEW_ARB_debug_output) {
        glDebugMessageCallbackARB((GLDEBUGPROCARB)&error_cb, NULL);
        il_log(3, "ARB_debug_output present, enabling advanced errors");
        IL_GRAPHICS_TESTERROR("glDebugMessageCallbackARB()");
    } else
        il_log(3, "ARB_debug_output missing");
#endif

    glfwSwapInterval(0); // 1:1 ratio of frames to vsyncs
}

static void event_setup()
{
    //ilG_queue = ilE_queue_new();
    ilE_register(il_queue, IL_GRAPHICS_TICK, ILE_DONTCARE, (ilE_callback)&global_draw, NULL);
    ilE_register(il_queue, IL_BASE_SHUTDOWN, ILE_DONTCARE, (ilE_callback)&quit, NULL);
    int hz = glfwGetWindowParam(GLFW_REFRESH_RATE);
    struct timeval *tv = calloc(1, sizeof(struct timeval));
    tv->tv_usec = hz>0? 1000000.0/hz : 1000000.0/60;
    ilE_timer(il_queue, ilE_new(IL_GRAPHICS_TICK, 0, NULL), tv); // kick off the draw loop
}

void ilG_init()
{
    /*keymap = calloc(1, sizeof(il_keymap));
    il_keymap_defaults(keymap);
    il_keymap_parse("keymap.ini", keymap);*/
    // setup our shader directory
    ilA_registerReadDir(il_fromC("shaders"),0);

    // Setup GL context (glfw, glew, etc.)
    context_setup();
    IL_GRAPHICS_TESTERROR("Unknown");
    
    // GL setup
    glClearColor(1.0, 0.41, 0.72, 1.0); // hot pink because why not
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    IL_GRAPHICS_TESTERROR("Error setting up screen");

    // generate ilG_material_default
    ilG_material_init();
    // generate primitive defaults
    ilG_shape_init();
    // generate default textures
    ilG_texture_init();
        
    // register events
    event_setup();
}

static void draw()
{
    if (!context) {
        il_log(0, "Nothing to do.");
        ilE_pushnew(il_queue, IL_BASE_SHUTDOWN, 0, NULL);
        return;
    }
    if (!glfwGetWindowParam(GLFW_OPENED)) {
        ilE_pushnew(il_queue, IL_BASE_SHUTDOWN, 0, NULL);
        return;
    }
    il_log(5, "Rendering frame");
    struct timeval tv;
    il_positionable* pos = NULL;
    ilG_trackiterator * iter = ilG_trackiterator_new(context);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, context->framebuffer);

    glClearColor(0.39, 0.58, 0.93, 1.0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gettimeofday(&tv, NULL);

    static GLenum drawbufs[] = {
        GL_COLOR_ATTACHMENT0,   // accumulation
        GL_COLOR_ATTACHMENT1,   // normal
        GL_COLOR_ATTACHMENT2,   // diffuse
        GL_COLOR_ATTACHMENT3    // specular
    };
    glDrawBuffers(4, &drawbufs[0]);

    while (ilG_trackIterate(iter)) {
        if (context->drawable != ilG_trackGetDrawable(iter)) {
            if (context->drawable && context->drawable->unbind)
                context->drawable->unbind(context, 
                    context->drawable->unbind_ctx);
            context->drawable = ilG_trackGetDrawable(iter);
            if (context->drawable && context->drawable->bind)
                context->drawable->bind(context, 
                    context->drawable->bind_ctx);
        }
        if (context->material != ilG_trackGetMaterial(iter)) {
            if (context->material && context->material->unbind)
                context->material->unbind(context, 
                    context->material->unbind_ctx);
            context->material = ilG_trackGetMaterial(iter);
            if (context->material && context->material->bind)
                context->material->bind(context, 
                    context->material->bind_ctx);
        }
        if (context->texture != ilG_trackGetTexture(iter)) {
            if (context->texture && context->texture->unbind)
                context->texture->unbind(context, 
                    context->texture->unbind_ctx);
            context->texture = ilG_trackGetTexture(iter);
            if (context->texture && context->texture->bind)
                context->texture->bind(context, 
                    context->texture->bind_ctx);
        }

        pos = ilG_trackGetPositionable(iter);

        if (context->drawable && context->drawable->update)
            context->drawable->update(context, pos, 
                context->drawable->update_ctx);
        if (context->material && context->material->update)
            context->material->update(context, pos, 
                context->material->update_ctx);
        if (context->texture && context->texture->update)
            context->texture->update(context, pos, 
                context->texture->update_ctx);

        if (context->drawable && context->drawable->draw)
            context->drawable->draw(context, pos, context->drawable->draw_ctx);
    }
    context->drawable = NULL;
    context->material = NULL;
    context->texture = NULL;

    // TODO: light drawing code here
    glBindFramebuffer(GL_FRAMEBUFFER, context->framebuffer);
}

static void fullscreenTexture()
{
    ilG_testError("Unknown");
    static int data[] = {
        1, 1,
        1, 0,
        0, 0,
        1, 1,
        0, 1,
        0, 0,
    };
    static GLuint vao, vbo = 0;
    if (!vbo) {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
        glVertexAttribPointer(ILG_ARRATTR_POSITION, 2, GL_INT, GL_FALSE, 0, NULL);
        glVertexAttribPointer(ILG_ARRATTR_TEXCOORD, 2, GL_INT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
        glEnableVertexAttribArray(ILG_ARRATTR_TEXCOORD);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ilG_testError("Error drawing fullscreen quad");
}

static void global_draw()
{
    //il_log(3, "Drawing window");
    ilG_testError("Unknown");
    static ilG_material* material = NULL;
    if (!material) {
        const char* unitlocs[] = {
            "tex",
            NULL
        };
        unsigned long unittypes[] = {
            ILG_TUNIT_NONE
        };
        material = ilG_material_new(IL_ASSET_READFILE("post.vert"), 
            IL_ASSET_READFILE("post.frag"), "Post Processing Shader", 
            "in_Position", "in_Texcoord", NULL, NULL, &unitlocs[0], &unittypes[0],
            NULL, NULL, NULL, NULL, NULL);
    }
    draw();
    // clean up the state
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    context->material = material;
    material->bind(context, material->bind_ctx);
    // no update() as we don't deal with positionables here
    // setup to do postprocessing
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, context->fbtextures[1]); // bind the accumulation buffer
    fullscreenTexture();
    material->unbind(context, material->unbind_ctx);
    ilG_testError("global_draw()");
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

