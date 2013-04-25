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
    OPT('r', "run",     required_argument, "Runs a Lua script") \
    OPT(0,   "scripts", required_argument, "Adds a directory to look for scripts")
static const char *optstring = "m:r:";

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
    IL_ARRAY(char*,) script_paths = {0};
    IL_ARRAY(char*,) module_paths = {0};

    int opt, idx, has_modules = 0, has_scripts = 0;
    opterr = 0; // we don't want to print an error if another package uses an option
    while ((opt = getopt_long(argc, argv, optstring, longopts, &idx)) != -1) {
        switch(opt) {
            case 0:
                if (strcmp(longopts[idx].name, "scripts") == 0) {
                    IL_APPEND(script_paths, strdup(optarg));
                    has_scripts = 1;
                }
                break;
            case 'm':
                IL_APPEND(module_paths, strdup(optarg));
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
    int i;
    for (i = 0; i < module_paths.length; i++) {
        load_modules(module_paths.data[i], argc, argv);
        free(module_paths.data[i]);
    }
    IL_FREE(module_paths);

    if (!has_scripts) {
        IL_APPEND(script_paths, "script");
    }
    
    ilS_script *s = ilS_new();
    for (i = 0; i < script_paths.length; i++) {
        ilS_addPath(s, script_paths.data[i]);
    }
    int found_bootstrap = 0;
    for (i = 0; i < script_paths.length; i++) {
        char path[strlen(script_paths.data[i]) + strlen("/bootstrap.lua") + 1];
        sprintf(path, "%s/bootstrap.lua", script_paths.data[i]);
        if (!access(path, F_OK)) { // returns 0 on success
            ilS_fromFile(s, path);
            found_bootstrap = 1;
            break;
        }
    }
    if (!found_bootstrap) {
        fprintf(stderr, "MAIN: Could not find bootstrap.lua\n");
        return 1;
    }
    int res = ilS_run(s);
    if (res != 0) {
        fprintf(stderr, "MAIN: %s\n", s->err);
        return 1;
    }
    for (i = 0; i < scripts.length; i++) {
        ilS_fromFile(s, scripts.data[i]);
        free(scripts.data[i]);
        res = ilS_run(s);
        if (res != 0) {
            fprintf(stderr, "MAIN: %s\n", s->err);
        }
    }
    /*for (i = 0; i < script_paths.length; i++) {
        free(script_paths.data[i]);
    }*/ // TODO: stop leaking memory here because of weird segfault bug
    IL_FREE(script_paths);
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
