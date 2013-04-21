#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include "docopt.inc"
#include "loader.h"
#include "common/event.h"
#include "script/script.h"

char *strdup(const char *str);

char *strtok_r(char *str, const char *delim, char **saveptr);

int main(int argc, char **argv)
{
    fprintf(stderr, "MAIN: Initializing engine.\n");

    DocoptArgs args = docopt(argc, argv, 1, "0.0pre-alpha");

#ifdef WIN32
    // I have no idea
    WSADATA WSAData;
    WSAStartup(0x101, &WSAData);
#endif

    // TODO: windows
    const char *modpath = "modules";
    if (args.path) {
        modpath = args.path;
    }
    DIR *dir = opendir(modpath); // TODO: more robust module loader
    if (!dir) {
        fprintf(stderr, "Failed to open modules directory: %s\n", strerror(errno));
        return 1;
    }
    struct dirent entry, *result;
    while (!readdir_r(dir, &entry, &result) && result) {
        if (strcmp(result->d_name + strlen(result->d_name) - 3, ".so") != 0) {
            // assume it's not a shared library if it doesn't end with .so
            continue;
        }
        char buf[512];
        snprintf(buf, 512, "%s/%s", modpath, result->d_name);
        il_loadmod(buf, argc, argv);
    }
    closedir(dir);

    if (args.run) {
        ilS_script *s = ilS_new();
        ilS_fromFile(s, "script/bootstrap.lua");
        int res = ilS_run(s);
        if (res != 0) {
            fprintf(stderr, "%s\n", s->err);
            return 1;
        }
        ilS_fromFile(s, args.run);
        res = ilS_run(s);
        if (res != 0) {
            fprintf(stderr, "%s\n", s->err);
        }
    }

    // main loop
    fprintf(stderr, "MAIN: Starting main loop\n");
    void (*loop)();
    loop = (void(*)())il_getsym("libilcommon", "ilE_loop");
    loop();

    return 0;
}
