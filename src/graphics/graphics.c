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
#include "common/world.h"
#include "graphics/drawable3d.h"
#include "graphics/material.h"
#include "graphics/glutil.h"
#include "util/ilstring.h"
#include "graphics/arrayattrib.h"
#include "graphics/textureunit.h"
#include "graphics/fragdata.h"
#include "graphics/bindable.h"

#define OPTIONS \
    OPT(0,   "shaders", required_argument, "Adds a directory to look for shaders")
static const char *optstring = "";

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

il_base *ilG_shaders_dir;
const ilA_dir *ilG_shaders_iface;

static void quit();

static void glew_error(int code, const char *err)
{
    (void)code;
    il_log_real("", 0, "GLFW", 1, "%s", err);
}

void ilG_registerJoystickBackend();
static void glfw_setup()
{
    glfwSetErrorCallback(glew_error); // unspecified
    if (!glfwInit()) { // main thread
        il_error("glfwInit() failed");
        abort();
    }

    il_log("Using GLFW version %s", glfwGetVersionString()); // thread safe
    ilG_registerJoystickBackend();
}

static void event_setup()
{
    ilE_register(ilE_shutdown, ILE_DONTCARE, ILE_MAIN, &quit, il_value_nil());
}

const char **il_dependencies(int argc, char **argv)
{
    (void)argc, (void)argv;
    static const char *deps[] = {
        "ilcommon",
        "ilutil",
        NULL
    };
    return deps;
}

void ilG_material_init();
void ilG_shape_init();
void ilG_quad_init();
int il_bootstrap(int argc, char **argv)
{
    int opt, idx;
    opterr = 0; // we don't want to print an error if another package uses an option
    optind = 0; // reset getopt
    while ((opt = getopt_long(argc, argv, optstring, longopts, &idx)) != -1) {
        switch(opt) {
            case 0:
            if (strcmp(longopts[idx].name, "shaders") == 0) {
                ilA_path *path = ilA_path_chars(optarg);
                const ilA_dir *iface;
                il_base *base = ilA_stdiodir(path, &iface);
                ilA_path_free(path);
                if (ilG_shaders_dir) {
                    ilG_shaders_dir = ilA_union(ilG_shaders_iface, iface, ilG_shaders_dir, base, &ilG_shaders_iface);
                } else {
                    ilG_shaders_dir = base;
                    ilG_shaders_iface = iface;
                }
            }
            break;
            case '?':
            default:
            break;
        }
    }
    if (!ilG_shaders_dir) {
        ilA_path *path = ilA_path_chars("shaders");
        ilG_shaders_dir = ilA_stdiodir(path, &ilG_shaders_iface);
        ilA_path_free(path);
    }

    glfw_setup();

    ilG_material_init();
    ilG_shape_init();
    ilG_quad_init();

    // register events
    event_setup();

    return 0;
}

static void quit(const il_value *data, il_value *ctx)
{
    (void)data, (void)ctx;
    glfwTerminate(); // main thread
}

