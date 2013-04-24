#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>

//#include "docopt.inc"
#include "loader.h"
#include "script/script.h"
#include "util/array.h" // entirely in preprocessor, so it's fine

#define OPTIONS \
    OPT('m', "modules", required_argument, "Adds a directory to look for modules") \
    OPT('r', "run", required_argument, "Runs a Lua script")
static const char *optstring = "m:r:";

#define OPT(s, l, a, h) {l, a, NULL, s},
static struct option longopts[] = {
    OPTIONS
};
#undef OPT

#define OPT(s, l, a, h) h,
static const char *help[] = {
    OPTIONS
};
#undef OPT

static void load_modules(const char *path, int argc, char **argv)
{
    // TODO: windows
    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "Failed to open modules directory: %s\n", strerror(errno));
    }
    struct dirent entry, *result;
    while (!readdir_r(dir, &entry, &result) && result) {
        if (strcmp(result->d_name + strlen(result->d_name) - 3, ".so") != 0) {
            // assume it's not a shared library if it doesn't end with .so
            continue;
        }
        char buf[512];
        snprintf(buf, 512, "%s/%s", path, result->d_name);
        il_loadmod(buf, argc, argv);
    }
    closedir(dir);
}

int main(int argc, char **argv)
{
    fprintf(stderr, "MAIN: Initializing engine.\n");

    IL_ARRAY(char*,) scripts = {0};

    int opt, idx, has_modules = 0;
    opterr = 0; // we don't want to print an error if another package uses an option
    while ((opt = getopt_long(argc, argv, optstring, longopts, &idx)) != -1) {
        switch(opt) {
            case 'm':
                load_modules(optarg, argc, argv);
                has_modules = 1;
                break;
            case 'r':
                IL_APPEND(scripts, strdup(optarg));
                break;
            case '?':
            default:
                break;
        }
    }

    if (!has_modules) {
        load_modules("modules", argc, argv); // default path
    }
    
    ilS_script *s = ilS_new();
    ilS_fromFile(s, "script/bootstrap.lua");
    int res = ilS_run(s);
    if (res != 0) {
        fprintf(stderr, "MAIN: %s\n", s->err);
        return 1;
    }
    int i;
    for (i = 0; i < scripts.length; i++) {
        ilS_fromFile(s, scripts.data[i]);
        free(scripts.data[i]);
        res = ilS_run(s);
        if (res != 0) {
            fprintf(stderr, "MAIN: %s\n", s->err);
        }
    }
    IL_FREE(scripts);

    // main loop
    fprintf(stderr, "MAIN: Starting main loop\n");
    void (*loop)();
    loop = (void(*)())il_getsym("libilcommon", "ilE_loop");
    if (!loop) {
        return 1;
    }
    loop();

    return 0;
}
