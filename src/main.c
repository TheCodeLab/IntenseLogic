#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "time.h"
#include <event2/event.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/glfw.h>
#include "docopt.inc"

#include "common/base.h"
#include "common/input.h"
#include "common/event.h"
#include "graphics/graphics.h"
#include "network/network.h"
//#include "physics/physics.h"
#include "script/script.h"
#include "asset/asset.h"
//#include "graphics/heightmap.h"
#include "graphics/drawable3d.h"
#include "common/log.h"
#include "common/entity.h"

extern struct event_base * ilE_base;
extern void il_init();

static void GLFWCALL key_cb(int key, int action)
{
    if (action == GLFW_PRESS)
        ilE_pushnew(IL_INPUT_KEYDOWN, sizeof(int), &key);
    else
        ilE_pushnew(IL_INPUT_KEYUP, sizeof(int), &key);
}

static void GLFWCALL mouse_cb(int button, int action)
{
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

static void update(ilE_event * ev, void * ctx)
{
    (void)ev;
    (void)ctx;
    glfwPollEvents();
    //il_log(5, "tick");
}

int running = 1;

void shutdown_callback(ilE_event* ev)
{
    (void)ev;
    il_log(3, "Shutting down.");
    event_base_loopbreak(ilE_base);
}

char *strtok_r(char *str, const char *delim, char **saveptr);

#ifdef WIN32
// http://stackoverflow.com/a/12979321
char *strtok_r(char *str, const char *delim, char **nextp)
{
    char *ret;

    if (str == NULL) {
        str = *nextp;
    }

    str += strspn(str, delim);

    if (*str == '\0') {
        return NULL;
    }

    ret = str;
    str += strcspn(str, delim);

    if (*str) {
        *str++ = '\0';
    }

    *nextp = str;
    return ret;
}
#endif

int main(int argc, char **argv)
{
    DocoptArgs args = docopt(argc, argv, 1, "0.0pre-alpha");

#if defined(WIN32) && !defined(DEBUG)
    il_logfile = fopen("nul", "w");
#else
    il_logfile = fopen("/dev/null", "w");
#endif

#ifdef DEBUG
    il_logfile = stdout;
    il_loglevel = IL_COMMON_LOGNOTICE;
#endif

    il_log(3, "Initialising engine.");

    // search path priority (lower to highest):
    // defaults, config files, environment variables, command line options

    // read environment variables
    char *path = getenv("IL_PATH");
    if (path) {
        char *saveptr = NULL;
        char *str = strdup(path);
        char *token;

        token = strtok_r(str, ":", &saveptr);
        while(token) {
            ilA_registerReadDir(il_fromC(token), 3);
            token = strtok_r(NULL, ":", &saveptr);
        }
    } else {
        // reasonable defaults
        ilA_registerReadDir(il_fromC((char*)"."), 4);
        ilA_registerReadDir(il_fromC((char*)"config"), 4);
        ilA_registerReadDir(il_fromC((char*)"shaders"), 4);
    }

    // build command line overrides
    int i = 0;
    const char * scripts[argc];
    int n_scripts = 0;

    if (args.logfile){
        il_logfile = fopen(optarg, "a");
    }

    if (args.verbose){
        il_loglevel = atoi(optarg)?atoi(optarg):4;
    }

    if (args.run){
        scripts[n_scripts] = optarg;
        n_scripts++;
    }

    if (args.path){
        ilA_registerReadDir(il_fromC(optarg), 1);
    }

    // build config file


    il_log(3, "Asset paths loaded");

    // I have no idea why I have to use this piece of code
#ifdef WIN32
    WSADATA WSAData;
    WSAStartup(0x101, &WSAData);
#endif

    // register glfw stuff
    glfwSetKeyCallback(&key_cb);
    glfwSetMouseButtonCallback(&mouse_cb);
    glfwSetMousePosCallback(&mousemove_cb);
    glfwSetMouseWheelCallback(&mousewheel_cb);

    // initialise engine
    il_init();

    // register the updater first so it gets called first (no prioritisation
    // system needed yet, or really important)
    ilE_register(IL_BASE_TICK, (ilE_callback)&update, NULL);

    ilN_init();
    ilG_init();
    //ilP_init();
    ilS_init();
    //ilA_init();
    ilE_register(IL_BASE_SHUTDOWN, (ilE_callback)&shutdown_callback, NULL);

    // finished initialising, send startup event
    ilE_pushnew(IL_BASE_STARTUP, 0, NULL);

    // Run startup scripts
    for (i = 0; i < n_scripts; i++) {
        ilS_loadfile(scripts[i]);
    }

    // main loop
    il_log(3, "Starting main loop");
    event_base_loop(ilE_base, 0);

    // shutdown code (only reached after receiving a IL_BASE_SHUTDOWN event)

    return 0;
}
