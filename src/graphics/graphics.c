#include "graphics.h"

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_video.h>
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
#include "util/logger.h"
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

static void sdl_error(void *ptr, int cat, SDL_LogPriority pri, const char *msg)
{
    (void)ptr;
    const char *scat;
    unsigned level;
    switch (cat) {
#define C(n, s) case n: scat = s; break;
        C(SDL_LOG_CATEGORY_APPLICATION, "application"   )
        C(SDL_LOG_CATEGORY_ERROR,       "error"         )
        C(SDL_LOG_CATEGORY_SYSTEM,      "system"        )
        C(SDL_LOG_CATEGORY_AUDIO,       "audio"         )
        C(SDL_LOG_CATEGORY_VIDEO,       "video"         )
        C(SDL_LOG_CATEGORY_RENDER,      "render"        )
        C(SDL_LOG_CATEGORY_INPUT,       "input"         )
        C(SDL_LOG_CATEGORY_CUSTOM,      "custom"        )
        default:
        scat = "unknown";
#undef C
    }
    switch (pri) {
#define C(n, l) case n: level = l; break;
        C(SDL_LOG_PRIORITY_VERBOSE,  5)
        C(SDL_LOG_PRIORITY_DEBUG,    4)
        C(SDL_LOG_PRIORITY_INFO,     3)
        C(SDL_LOG_PRIORITY_WARN,     2)
        C(SDL_LOG_PRIORITY_ERROR,    1)
        C(SDL_LOG_PRIORITY_CRITICAL, 0)
        default:
        level = 3;
#undef C
    }
    il_logmsg *log = il_logmsg_new(1);
    il_logmsg_setLevel(log, level);
    char buf[64];
    sprintf(buf, "SDL %s error", scat);
    il_logmsg_copyMessage(log, msg);
    il_logmsg_copyBtString(log, 0, buf);
    il_logger_log(il_logger_stderr, log); // TODO: Log to appropriate location
}

static void sdl_setup()
{
    if (SDL_Init(SDL_INIT_NOPARACHUTE) != 0) {
        il_error("SDL_Init: %s", SDL_GetError());
    }
    if (SDL_VideoInit(NULL) != 0) {
        il_error("SDL_VideoInit: %s", SDL_GetError());
    }
    SDL_LogSetOutputFunction(sdl_error, NULL);
    il_log("Using SDL %s", SDL_GetRevision());
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

    //glfw_setup();
    sdl_setup();

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
    SDL_VideoQuit();
    SDL_Quit();
}

