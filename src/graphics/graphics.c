#include "graphics.h"

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>

#include "graphics/camera.h"
#include "common/event.h"
#include "common/input.h"
#include "math/matrix.h"
#include "common/base.h"
//#include "common/keymap.h"
#include "graphics/context.h"
#include "util/log.h"
#include "graphics/shape.h"
#include "asset/asset.h"
#include "common/world.h"
#include "graphics/drawable3d.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "graphics/tracker.h"
#include "graphics/glutil.h"
#include "util/ilstring.h"
#include "graphics/arrayattrib.h"
#include "graphics/textureunit.h"
#include "graphics/fragdata.h"
#include "graphics/bindable.h"

#define OPTIONS \
    OPT(0,   "shaders", required_argument, "Adds a directory to look for shaders") \
    OPT('w', "width",   required_argument, "Sets the window width") \
    OPT('h', "height",  required_argument, "Sets the window height")
static const char *optstring = "w:h:";

#define OPT(s, l, a, h) {l, a, NULL, s},
static struct option longopts[] = {
    OPTIONS
    {0, 0, NULL, 0}
};
#undef OPT

#define OPT(s, l, a, h) h,
static const char *help[] = {
    OPTIONS
};
#undef OPT

static int width = 800;
static int height = 600;
static ilG_context* context = NULL;
ilE_registry *ilG_registry;

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
    il_debug("Key %c", key);
    if (action == GLFW_PRESS) {
        ilE_globalevent(il_registry, "input.keydown", sizeof(int), &key);
    } else {
        ilE_globalevent(il_registry, "input.keyup", sizeof(int), &key);
    }
}

static void GLFWCALL mouse_cb(int button, int action)
{
    il_debug("Mouse %i", button);
    if (action == GLFW_PRESS) {
        ilE_globalevent(il_registry, "input.mousedown", sizeof(int), &button);
    } else {
        ilE_globalevent(il_registry, "input.mouseup", sizeof(int), &button);
    }
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
    ilE_globalevent(il_registry, "input.mousemove", sizeof(ilI_mouseMove), &mousemove);
}

static void GLFWCALL mousewheel_cb(int pos)
{
    ilI_mouseWheel mousewheel =
    (ilI_mouseWheel) {
        0, pos
    };
    ilE_globalevent(il_registry, "input.mousewheel", sizeof(ilI_mouseWheel), &mousewheel);
}

static void context_setup()
{
    if (!glfwInit()) {
        il_fatal("glfwInit() failed");
    }

    {
        int major, minor, rev;
        glfwGetVersion(&major, &minor, &rev);
        il_log("Using GLFW version %i.%i.%i", major, minor, rev);
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
        il_fatal("glfwOpenWindow() failed - are you sure you have OpenGL 3.1?");
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
        il_fatal("glewInit() failed: %s", glewGetErrorString(err));
    }
    il_log("Using GLEW %s", glewGetString(GLEW_VERSION));

    IL_GRAPHICS_TESTERROR("glewInit()");

#ifndef __APPLE__
    if (!GLEW_VERSION_3_1) {
        il_error("GL version 3.1 is required, your Segfault Insurance is now invalid");
    }
#endif

    IL_GRAPHICS_TESTERROR("Unknown");
#ifdef DEBUG
    if (GLEW_ARB_debug_output) {
        glDebugMessageCallbackARB((GLDEBUGPROCARB)&error_cb, NULL);
        glEnable(GL_DEBUG_OUTPUT);
        il_log("ARB_debug_output present, enabling advanced errors");
        IL_GRAPHICS_TESTERROR("glDebugMessageCallbackARB()");
    } else
        il_log("ARB_debug_output missing");
#endif

    glfwSwapInterval(0); // 1:1 ratio of frames to vsyncs
}

static void event_setup()
{
    ilG_registry = ilE_registry_new();
    ilE_register(ilG_registry, "tick", ILE_DONTCARE, ILE_MAIN, &global_draw, NULL);
    ilE_register(il_registry, "shutdown", ILE_DONTCARE, ILE_MAIN, &quit, NULL);
    int hz = glfwGetWindowParam(GLFW_REFRESH_RATE);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = hz>0? 1000000.0/hz : 1000000.0/60;
    ilE_globaltimer(ilG_registry, "tick", 0, NULL, tv); // kick off the draw loop
}

int il_bootstrap(int argc, char **argv)
{
    int opt, idx, has_shaders = 0;
    opterr = 0; // we don't want to print an error if another package uses an option
    optind = 0; // reset getopt
    while ((opt = getopt_long(argc, argv, optstring, longopts, &idx)) != -1) {
        switch(opt) {
            case 0:
                if (strcmp(longopts[idx].name, "shaders") == 0) {
                    ilA_registerReadDir(il_string_new(optarg, strlen(optarg)), 0);
                    has_shaders = 1;
                }
                break;
            case 'w':
                width = atoi(optarg);
                break;
            case 'h':
                height = atoi(optarg);
                break;
            case '?':
            default:
                break;
        }
    }
    if (!has_shaders) {
        // guess at the location of our shaders
        ilA_registerReadDir(il_string_new("shaders", strlen("shaders")),0);
    }

    // Setup GL context (glfw, glew, etc.)
    context_setup();
    IL_GRAPHICS_TESTERROR("Unknown");
    
    // GL setup
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

    return 1;
}

static void draw_geometry()
{
    struct timeval tv;
    il_positionable* pos = NULL;
    ilG_trackiterator * iter = ilG_trackiterator_new(context);
    gettimeofday(&tv, NULL);
    const ilG_bindable *drawable = NULL, *material = NULL, *texture = NULL;

    while (ilG_trackIterate(iter)) {
        pos = ilG_trackGetPositionable(iter);
        context->positionable = pos;

        ilG_bindable_swap(&drawable, (void**)&context->drawable, ilG_trackGetDrawable(iter));
        ilG_bindable_swap(&material, (void**)&context->material, ilG_trackGetMaterial(iter));
        ilG_bindable_swap(&texture,  (void**)&context->texture,  ilG_trackGetTexture(iter));

        ilG_bindable_action(material, context->material);
        ilG_bindable_action(texture,  context->texture);
        ilG_bindable_action(drawable, context->drawable);
    }
    context->drawable = NULL;
    context->material = NULL;
    context->texture = NULL;
}

static void draw_lights()
{ 
    ilG_testError("Unknown");
    if (!context->lightdata.material) {
        struct ilG_material* mtl = ilG_material_new();
        ilG_material_vertex(mtl, IL_ASSET_READFILE("light.vert"));
        ilG_material_fragment(mtl, IL_ASSET_READFILE("light.frag"));
        ilG_material_name(mtl, "Deferred Shader");
        ilG_material_arrayAttrib(mtl, ILG_ARRATTR_POSITION, "in_Position");
        ilG_material_textureUnit(mtl, ILG_TUNIT_NONE, "depth");
        ilG_material_textureUnit(mtl, ILG_TUNIT_NONE, "normal");
        ilG_material_textureUnit(mtl, ILG_TUNIT_NONE, "diffuse");
        ilG_material_textureUnit(mtl, ILG_TUNIT_NONE, "specular");
        ilG_material_matrix(mtl, ILG_INVERSE | ILG_VP, "ivp");
        ilG_material_fragData(mtl, ILG_FRAGDATA_ACCUMULATION, "out_Color");
        ilG_material_matrix(mtl, ILG_MVP, "mvp");
        if (ilG_material_link(mtl, context)) {
            abort();
        }
        context->lightdata.material = mtl;
        context->lightdata.invalidated = 1;
    }
    context->material = context->lightdata.material;
    /*if (context->lightdata.invalidated) {
        if (!context->lightdata.created) {
            context->lightdata.lights_index = glGetUniformBlockIndex(context->material->program, "LightBlock");
            context->lightdata.mvp_index = glGetUniformBlockIndex(context->material->program, "MVPBlock");
            glGetActiveUniformBlockiv(context->material->program, context->lightdata.lights_index, GL_UNIFORM_BLOCK_DATA_SIZE, &context->lightdata.lights_size);
            glGetActiveUniformBlockiv(context->material->program, context->lightdata.mvp_index, GL_UNIFORM_BLOCK_DATA_SIZE, &context->lightdata.mvp_size);
            //GLubyte *lights_buf = malloc(lights_size), *mvp_buf = malloc(mvp_size);
            const GLchar *lights_names[] = {
                "position",
                "color",
                "radius"
            }, *mvp_names[] = {
                "mvp"
            };
            GLuint lights_indices[3], mvp_indices[1];
            glGetUniformIndices(context->material->program, 3, lights_names, lights_indices);
            glGetUniformIndices(context->material->program, 1, mvp_names, mvp_indices);
            glGetActiveUniformsiv(context->material->program, 3, lights_indices, GL_UNIFORM_OFFSET, context->lightdata.lights_offset);
            glGetActiveUniformsiv(context->material->program, 1, mvp_indices, GL_UNIFORM_OFFSET, context->lightdata.mvp_offset);
            context->lightdata.created = 1;
        }
        if (context->lightdata.lights_ubo) {
            glDeleteBuffers(1, &context->lightdata.lights_ubo);
            glDeleteBuffers(1, &context->lightdata.mvp_ubo);
        }
        glGenBuffers(1, &context->lightdata.lights_ubo);
        glGenBuffers(1, &context->lightdata.mvp_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, context->lightdata.lights_ubo);
        GLubyte *lights_buf = calloc(1, context->lightdata.lights_size);        
        unsigned int i;
        for (i = 0; i < context->lights.length; i++) {
            ((il_Vector3*)lights_buf + context->lightdata.lights_offset[0])[i] = context->lights.data[i]->positionable->position;
            ((il_Vector3*)lights_buf + context->lightdata.lights_offset[1])[i] = context->lights.data[i]->color;
            ((float*)lights_buf + context->lightdata.lights_offset[2])[i] = context->lights.data[i]->radius;
        }
        glBufferData(GL_UNIFORM_BUFFER, context->lightdata.lights_size, lights_buf, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, context->lightdata.mvp_ubo);
        free(lights_buf);
        GLubyte *mvp_buf = calloc(1, context->lightdata.mvp_size);
        for (i = 0; i < context->lights.length; i++) {
            ((il_Matrix*)mvp_buf + context->lightdata.mvp_offset[0])[i] = 
                il_Matrix_transpose(ilG_computeMVP(context->camera, context->lights.data[i]->positionable));
        }
        glBufferData(GL_UNIFORM_BUFFER, context->lightdata.mvp_size, mvp_buf, GL_DYNAMIC_DRAW);
        free(mvp_buf);
        context->lightdata.invalidated = 0;
    }*/
    //glBindBufferBase(GL_UNIFORM_BUFFER, context->lightdata.lights_index, context->lightdata.lights_ubo);
    //glBindBufferBase(GL_UNIFORM_BUFFER, context->lightdata.mvp_index, context->lightdata.mvp_ubo);
    context->drawable = ilG_icosahedron;
    const ilG_bindable *drawable = il_cast(il_typeof(context->drawable), "il.graphics.bindable"),
                       *material = il_cast(il_typeof(context->material), "il.graphics.bindable");
    ilG_bindable_bind(drawable, context->drawable);
    ilG_bindable_bind(material, context->material);
    glUniform3f(glGetUniformLocation(context->material->program, "camera"), 
            context->camera->positionable.position[0], 
            context->camera->positionable.position[1],
            context->camera->positionable.position[2]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, context->fbtextures[0]);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_RECTANGLE, context->fbtextures[2]);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_RECTANGLE, context->fbtextures[3]);
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_RECTANGLE, context->fbtextures[4]);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    
    GLint position_loc  = glGetUniformLocation(context->material->program, "position"),
          color_loc     = glGetUniformLocation(context->material->program, "color"),
          radius_loc    = glGetUniformLocation(context->material->program, "radius");
    unsigned int i;
    for (i = 0; i < context->lights.length; i++) {
        context->positionable = &context->lights.data[i]->positionable;
        ilG_bindable_action(material, context->material);
        il_vec4 pos = context->positionable->position;
        glUniform3f(position_loc, pos[0], pos[1], pos[2]);
        il_vec4 col = context->lights.data[i]->color;
        glUniform3f(color_loc, col[0], col[1], col[2]);
        glUniform1f(radius_loc, context->lights.data[i]->radius);
        ilG_bindable_action(drawable, context->drawable);
        //glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(GLuint), GL_UNSIGNED_INT, NULL);
    }
    //glDrawElementsInstanced(GL_TRIANGLES, sizeof(indices)/sizeof(GLuint), GL_UNSIGNED_INT, NULL, context->lights.length);
    glDisable(GL_BLEND);
    ilG_bindable_unbind(drawable, context->drawable);
    ilG_bindable_unbind(material, context->material);
    context->drawable = NULL;
    context->material = NULL;
    ilG_testError("Error drawing lights");
}

static void draw()
{
    if (!context) {
        il_fatal("Nothing to render.");
        ilE_globalevent(il_registry, "shutdown", 0, NULL);
        return;
    }
    il_debug("Rendering frame");

    static GLenum drawbufs[] = {
        GL_COLOR_ATTACHMENT0,   // accumulation
        GL_COLOR_ATTACHMENT1,   // normal
        GL_COLOR_ATTACHMENT2,   // diffuse
        GL_COLOR_ATTACHMENT3    // specular
    };
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, context->framebuffer);
    glDrawBuffers(4, &drawbufs[0]);
    glClearColor(0.39, 0.58, 0.93, 1.0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_geometry();

    glBindFramebuffer(GL_FRAMEBUFFER, context->framebuffer);
    glFrontFace(GL_CCW);
    glCullFace(GL_FRONT);
    draw_lights();
    glCullFace(GL_BACK);
}

static void fullscreenTexture()
{

    ilG_testError("Unknown");
    static float data[] = {
        0, 0,
	0, 1,
	1, 1,
	1, 0
    };
    static GLuint vao, vbo = 0;
    if (!vbo) {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
        glVertexAttribPointer(ILG_ARRATTR_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glVertexAttribPointer(ILG_ARRATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
        glEnableVertexAttribArray(ILG_ARRATTR_TEXCOORD);
    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    ilG_testError("Error drawing fullscreen quad");
}

static void global_draw(const ilE_registry* registry, const char *name, size_t size, const void *data, void * ctx)
{
    (void)registry, (void)name, (void)size, (void)data, (void)ctx;
    if (!glfwGetWindowParam(GLFW_OPENED)) {
        ilE_globalevent(il_registry, "shutdown", 0, NULL);
        return;
    }
    //il_log(3, "Drawing window");
    ilG_testError("Unknown");
    static ilG_material* material = NULL;
    if (!material) {
        /*const char* unitlocs[] = {
            "tex",
            NULL
        };
        unsigned long unittypes[] = {
            ILG_TUNIT_NONE
        };
        material = ilG_material_new(IL_ASSET_READFILE("post.vert"), 
            IL_ASSET_READFILE("post.frag"), "Post Processing Shader", 
            "in_Position", "in_Texcoord", NULL, NULL, &unitlocs[0], &unittypes[0],
            NULL, NULL, NULL, NULL, NULL);*/
        material = ilG_material_new();
        ilG_material_vertex(material, IL_ASSET_READFILE("post.vert"));
        ilG_material_fragment(material, IL_ASSET_READFILE("post.frag"));
        ilG_material_name(material, "Post Processing Shader");
        ilG_material_arrayAttrib(material, ILG_ARRATTR_POSITION, "in_Position");
        ilG_material_arrayAttrib(material, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
        ilG_material_textureUnit(material, ILG_TUNIT_NONE, "tex");
        if (ilG_material_link(material, context)) {
            abort();
        }
	ilG_testError("Error creating material");
    }
    glClearColor(1.0, 0.41, 0.72, 1.0); // hot pink because why not
    ilG_testError("glClearColor");
    glClearDepth(1.0);
    ilG_testError("glClearDepth");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ilG_testError("glClear");
    glEnable(GL_CULL_FACE);
    ilG_testError("glEnable");
    ilG_testError("Error setting up for draw");
    draw();
    ilG_testError("Error drawing scene");
    // clean up the state
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    context->material = material;
    ilG_bindable_bind(il_cast(il_typeof(material), "il.graphics.bindable"), material);
    // no update() as we don't deal with positionables here
    // setup to do postprocessing
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, context->fbtextures[1]); // bind the accumulation buffer
    ilG_testError("Error setting up for post processing");
    fullscreenTexture();
    ilG_testError("Error post processing");
    ilG_bindable_unbind(il_cast(il_typeof(material), "il.graphics.bindable"), material);
    ilG_testError("Error cleaning up shaders");
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
    il_log("OpenGL %s %s (%s): %s\n", ssource, stype,
            sseverity, message);
}

static void quit(const ilE_registry* registry, const char *name, size_t size, const void *data, void * ctx)
{
    (void)registry, (void)name, (void)size, (void)data, (void)ctx;
    glfwTerminate();
}

