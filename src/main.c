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
#include "script.h"
#include "util/array.h" // entirely in preprocessor, so it's fine
#include "version.h"

#define OPTIONS \
    OPT('m', "modules", required_argument,  "Adds a directory to look for modules") \
    OPT('i', "ignore",  required_argument,  "Ignores a module while loading") \
    OPT('r', "run",     required_argument,  "Runs a Lua script") \
    OPT(0,   "scripts", required_argument,  "Adds a directory to look for scripts") \
    OPT('h', "help",    no_argument,        "Prints this message and exits") \
    OPT('v', "version", no_argument,        "Prints the version and exits")
static const char *optstring = "m:r:hvi:";

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

char *strdup(const char*);
int main(int argc, char **argv)
{
    IL_ARRAY(char*,) scripts = {0,0,0};
    IL_ARRAY(char*,) script_paths = {0,0,0};
    int opt, idx, has_modules = 0, has_scripts = 0, found_bootstrap = 0, res;
    size_t i;
    ilS_script *s = ilS_new();
    void (*loop)();

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
            il_add_module_path(optarg); //IL_APPEND(module_paths, strdup(optarg));
            has_modules = 1;
            break;
            case 'r':
            IL_APPEND(scripts, strdup(optarg));
            break;
            case 'i':
            il_ignore_module(optarg);
            break;
            case 'h':
            printf("IntenseLogic %s\n", il_version);
            printf("Usage: %s [OPTIONS]\n\n", argv[0]);
            printf("Each module may have its own options, see relavent documentation for those.\n\n");
            printf("Options:\n");
            for (i = 0; longopts[i].name; i++) {
                printf(" %c%c %s%-12s %s\n", longopts[i].val? '-' : ' ', 
                       longopts[i].val? longopts[i].val : ' ',
                       longopts[i].name? "--" : "  ",
                       longopts[i].name? longopts[i].name : "",
                       help[i]
                );
            }
            return 0;
            case 'v':
            printf("IntenseLogic %s\n", il_version);
            printf("Built %s\n", __DATE__);
            return 0;
            case '?':
            default:
            break;
        }
    }

    fprintf(stderr, "MAIN: Initializing engine.\n");
    fprintf(stderr, "MAIN: IntenseLogic %s\n", il_version);
    fprintf(stderr, "MAIN: Built %s\n", __DATE__);

    if (!has_modules) {
        il_load_module_dir("modules", argc, argv); // default path
    }
    il_load_module_paths(argc, argv);

    if (!has_scripts) {
        IL_APPEND(script_paths, "script");
    }
    
    for (i = 0; i < script_paths.length; i++) {
        ilS_addPath(s, script_paths.data[i]);
    }
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
    res = ilS_run(s);
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
    loop = (void(*)())il_get_symbol("ilcommon", "ilE_loop");
    if (!loop) {
        return 1;
    }
    loop();

    return 0;
}
