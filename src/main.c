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
#include "util/array.h" // entirely in preprocessor, so it's fine
#include "version.h"

#define OPTIONS \
    OPT('m', "modules", required_argument,  "Adds a directory to look for modules") \
    OPT('i', "ignore",  required_argument,  "Ignores a module while loading") \
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
    int opt, idx, has_modules = 0;
    size_t i;
    void (*loop)();
    void (*quit)();

    opterr = 0; // we don't want to print an error if another package uses an option
    while ((opt = getopt_long(argc, argv, optstring, longopts, &idx)) != -1) {
        switch(opt) {
            case 'm':
            il_add_module_path(optarg); //IL_APPEND(module_paths, strdup(optarg));
            has_modules = 1;
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
   
    // main loop
    fprintf(stderr, "MAIN: Starting main loop\n");
    loop = (void(*)())il_get_symbol("ilcommon", "ilE_loop");
    if (!loop) {
        return 1;
    }
    loop();
    quit = (void(*)())il_get_symbol("ilcommon", "ilE_quit");
    quit();

    return 0;
}
