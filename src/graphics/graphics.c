#include "graphics.h"

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>

#include "graphics/camera.h"
#include "common/event.h"
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

// TODO: help options for modules
/*#define OPT(s, l, a, h) h,
static const char *help[] = {
    OPTIONS
};
#undef OPT*/

static int width = 800;
static int height = 600;
ilE_registry *ilG_registry;

static void quit();
void ilG_material_init();
void ilG_shape_init();
void ilG_quad_init();
void ilG_texture_init();

/*static void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    il_debug("Key %c", key);
    if (action == GLFW_PRESS) {
        ilE_globalevent(il_registry, "input.keydown", sizeof(int), &key);
    } else {
        ilE_globalevent(il_registry, "input.keyup", sizeof(int), &key);
    }
}

static void mouse_cb(int button, int action)
{
    il_debug("Mouse %i", button);
    if (action == GLFW_PRESS) {
        ilE_globalevent(il_registry, "input.mousedown", sizeof(int), &button);
    } else {
        ilE_globalevent(il_registry, "input.mouseup", sizeof(int), &button);
    }
}

static void mousemove_cb(int x, int y)
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
}*/

static void glfw_setup()
{
    if (!glfwInit()) {
        il_fatal("glfwInit() failed");
    }

    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    il_log("Using GLFW version %i.%i.%i", major, minor, rev);

    glfwSwapInterval(0); // 1:1 ratio of frames to vsyncs
}

static void update(const ilE_registry* registry, const char *name, size_t size, const void *data, void * ctx)
{
    (void)registry, (void)name, (void)size, (void)data, (void)ctx;
    glfwPollEvents();
}

static void event_setup()
{
    ilG_registry = ilE_registry_new();
    ilE_register(ilG_registry, "tick", ILE_DONTCARE, ILE_MAIN, update, NULL);
    ilE_register(il_registry, "shutdown", ILE_DONTCARE, ILE_MAIN, &quit, NULL);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000000.0/60; // TODO: get some proper refresh rate code instead of hardcoding 60fps
    ilE_globaltimer(ilG_registry, "tick", 0, NULL, tv); // kick off the draw loop
}

const char **il_dependencies(int argc, char **argv)
{
    (void)argc, (void)argv;
    static const char *deps[] = {
        "ilcommon",
        NULL
    };
    return deps;
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

    glfw_setup();
    IL_GRAPHICS_TESTERROR("Unknown");
    
    // GL setup
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    IL_GRAPHICS_TESTERROR("Error setting up screen");

    // generate ilG_material_default
    ilG_material_init();
    // generate primitive defaults
    ilG_shape_init();
    // generate gui quad
    ilG_quad_init();
    // generate default textures
    ilG_texture_init();
        
    // register events
    event_setup();

    return 1;
}

static void quit(const ilE_registry* registry, const char *name, size_t size, const void *data, void * ctx)
{
    (void)registry, (void)name, (void)size, (void)data, (void)ctx;
    glfwTerminate();
}

