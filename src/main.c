#include <string.h>
#include <stdlib.h>
#include "docopt.inc"
#include "common/common.h"
#include "util/log.h"
#include <GL/glfw.h>
#include <event2/event.h>
#include <string.h>

static void update(ilE_event * ev, void * ctx)
{
    (void)ev;
    (void)ctx;
    glfwPollEvents();
    il_debug("tick");
}

int running = 1;

void ilE_loop();

char *strdup(const char *str);

char *strtok_r(char *str, const char *delim, char **saveptr);

int main(int argc, char **argv)
{
    DocoptArgs args = docopt(argc, argv, 1, "0.0pre-alpha");

    il_log("Initialising engine.");

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
            ilA_registerReadDir(il_string_new(token, strlen(token)), 3);
            token = strtok_r(NULL, ":", &saveptr);
        }
    } else {
        // reasonable defaults
        ilA_registerReadDir(il_string_new(".",       -1), 4);
        ilA_registerReadDir(il_string_new("config",  -1), 4);
        ilA_registerReadDir(il_string_new("shaders", -1), 4);
    }
    il_log("Asset paths loaded");

    /*if (args.logfile){
        il_logfile = fopen(args.logfile, "a");
    }

    if (args.verbose){
        il_loglevel = atoi(args.verbose);
    }*/

    if (args.path){
        ilA_registerReadDir(il_string_new(args.path, strlen(args.path)), 1);
    }

    // I have no idea why I have to use this piece of code
#ifdef WIN32
    WSADATA WSAData;
    WSAStartup(0x101, &WSAData);
#endif

    // initialise engine
    il_init(args);

    ilE_register(il_queue, IL_BASE_TICK, ILE_BEFORE, (ilE_callback)&update, NULL);
    // finished initialising, send startup event
    ilE_pushnew(il_queue, IL_BASE_STARTUP, 0, NULL);

    if (args.run) {
        ilS_loadfile(args.run);
    }

    // main loop
    il_log("Starting main loop");
    ilE_loop();

    // shutdown code (only reached after receiving a IL_BASE_SHUTDOWN event)

    return 0;
}
