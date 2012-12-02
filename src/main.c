#include <stdlib.h>
#include <GL/glfw.h>
#include "docopt.inc"
#include "common/common.h"

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
    il_log(3, "Asset paths loaded");

    if (args.logfile){
        il_logfile = fopen(args.logfile, "a");
    }

    if (args.verbose){
        il_loglevel = IL_COMMON_LOGDEBUG;
    }

    if (args.path){
        ilA_registerReadDir(il_fromC(args.path), 1);
    }

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
    ilE_register(IL_BASE_SHUTDOWN, (ilE_callback)&shutdown_callback, NULL);

    // finished initialising, send startup event
    ilE_pushnew(IL_BASE_STARTUP, 0, NULL);

    if (args.run) {
        ilS_loadfile(args.run);
    }

    // main loop
    il_log(3, "Starting main loop");
    event_base_loop(ilE_base, 0);

    // shutdown code (only reached after receiving a IL_BASE_SHUTDOWN event)

    return 0;
}
