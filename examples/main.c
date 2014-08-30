#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util/version.h"
#include "util/loader.h"
#include "util/log.h"
#include "util/opt.h"
#include "graphics/graphics.h"

struct {
    enum {
        NO_ARG,
        REQUIRED,
        OPTIONAL
    } arg;
    char s, *l, *h;
} help[] = {
    {REQUIRED,  'm', "modules", "Adds a directory to look for modules"},
    {REQUIRED,  'i', "ignore",  "Ignores a module while loading"},
    {NO_ARG,    'h', "help",    "Prints this message and exits"},
    {NO_ARG,    'v', "version", "Prints the version and exits"},
    {REQUIRED,  's', "shaders", "Adds a directory to look for shaders"},
    {NO_ARG,      0, NULL,      NULL}
};

char *strdup(const char*);
char *strndup(const char*, size_t);
int main(int argc, char **argv)
{
    int has_modules = 0;
    size_t i;
    void (*quit)();
    il_opts opts = il_opt_parse(argc, argv);
    il_modopts *main_opts = il_opts_lookup(&opts, "");

    for (i = 0; main_opts && i < main_opts->args.length; i++) {
        il_opt *opt = &main_opts->args.data[i];
        char *arg = strndup(opt->arg.str, opt->arg.len);
#define option(s, l) if (il_string_cmp(opt->name, il_string_new(s)) || il_string_cmp(opt->name, il_string_new(l)))
        option("m", "modules") {
            il_add_module_path(arg);
            has_modules = 1;
        }
        option("i", "ignore") {
            il_ignore_module(arg);
        }
        option("h", "help") {
            printf("IntenseLogic %s\n", il_version);
            printf("Usage: %s [OPTIONS]\n\n", argv[0]);
            printf("Each module may have its own options, see relavent documentation for those.\n\n");
            printf("Options:\n");
            for (i = 0; help[i].l; i++) {
                static const char *arg_strs[] = {
                    "",
                    "[=arg]",
                    "=arg"
                };
                char longbuf[64];
                sprintf(longbuf, "%s%s%s",
                        help[i].l? "-" : " ",
                        help[i].l? help[i].l : "",
                        arg_strs[help[i].arg]
                );
                printf(" %c%c %-18s %s\n",
                       help[i].s? '-' : ' ',
                       help[i].s? help[i].s : ' ',
                       longbuf,
                       help[i].h
                );
            }
            return 0;
        }
        option("v", "version") {
            printf("IntenseLogic %s\n", il_version);
            printf("Commit: %s\n", il_commit);
            printf("Built %s\n", il_build_date);
            return 0;
        }
        option("s", "shaders") {
            ilG_shaders_addPath(arg);
        }
        free(arg);
    }

    il_log("Initializing engine.");
    il_log("IntenseLogic %s", il_version);
    il_log("Built %s", __DATE__);

    void il_load_ilgraphics();
    il_load_ilgraphics();

    if (!has_modules) {
        il_load_module_dir("modules", &opts); // default path
    }
    il_load_module_paths(&opts);
    il_postload_all();

    //
    // MAIN LOOP GOES HERE
    // Call ilG_sdlPollEvents regularly from the main loop.
    // TODO: Example of setting up graphics
    //

    void ilG_quit();
    ilG_quit();

    return 0;
}
