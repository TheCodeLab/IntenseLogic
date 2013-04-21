#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "docopt.inc"
#include "loader.h"
#include "asset/asset.h"
#include "common/event.h"
#include "script/script.h"

void ilE_loop();

char *strdup(const char *str);

char *strtok_r(char *str, const char *delim, char **saveptr);

int main(int argc, char **argv)
{
    fprintf(stderr, "MAIN: Initializing engine.\n");

    DocoptArgs args = docopt(argc, argv, 1, "0.0pre-alpha");

    // search path priority (lower to highest):
    // defaults, config files, environment variables, command line options

    // read environment variables
    // TODO: Move this into asset's bootstrap
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
    //il_log("Asset paths loaded");

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

    // TODO: windows
    DIR *dir = opendir("modules"); // TODO: more robust module loader
    struct dirent entry, *result;
    while (!readdir_r(dir, &entry, &result) && result) {
        if (strcmp(result->d_name + strlen(result->d_name) - 3, ".so") != 0) {
            // assume it's not a shared library
            continue;
        }
        char buf[512];
        snprintf(buf, 512, "modules/%s", result->d_name);
        il_loadmod(buf, argc, argv);
    }
    closedir(dir);

    // TODO: move to common bootstrap
    // finished initialising, send startup event
    ilE_globalevent(il_registry, "startup", 0, NULL);

    if (args.run) {
        ilS_loadfile(args.run);
    }

    // main loop
    fprintf(stderr, "MAIN: Starting main loop\n");
    ilE_loop();

    return 0;
}
