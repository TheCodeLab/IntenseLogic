#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util/loader.h"
#include "util/version.h"
#include "util/opt.h"
#include "asset/node.h"

const struct {
    enum {
        NO_ARG,
        REQUIRED,
        OPTIONAL
    } arg;
    char s;
    const char *l, *h;
} help[] = {
    {NO_ARG,    'h', "help",    "Prints this message and exits"},
    {NO_ARG,    'v', "version", "Prints the version and exits"},
    {REQUIRED,  'd', "data",    "Adds a directory to look for data files"},
    {NO_ARG,      0, NULL,      NULL}
};

ilA_fs demo_fs;

void demo_start();

char *strdup(const char*);
char *strndup(const char*, size_t);
int main(int argc, char **argv)
{
    size_t i;
    il_opts opts = il_opt_parse(argc, argv);
    il_modopts *main_opts = il_opts_lookup(&opts, "");

    ilA_adddir(&demo_fs, ".", -1);

    for (i = 0; main_opts && i < main_opts->args.length; i++) {
        il_opt *opt = &main_opts->args.data[i];
        char *arg = strndup(opt->arg.str, opt->arg.len);
#define option(s, l) if (il_string_cmp(opt->name, il_string_new(s)) || il_string_cmp(opt->name, il_string_new(l)))
        option("h", "help") {
            printf("IntenseLogic %s\n", il_version);
            printf("Usage: %s [OPTIONS]\n\n", argv[0]);
            printf("Each module may have its own options, see relavent documentation for those.\n\n");
            printf("Options:\n");
            for (i = 0; help[i].l; i++) {
                static const char *const arg_strs[] = {
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
            printf("Built: %s\n", il_build_date);
            return 0;
        }
        option("d", "data") {
            ilA_adddir(&demo_fs, arg, -1);
        }
        free(arg);
    }

    fprintf(stderr, "MAIN: Initializing engine.\n");
    fprintf(stderr, "MAIN: IntenseLogic %s\n", il_version);
    fprintf(stderr, "MAIN: IL Commit: %s\n", il_commit);
    fprintf(stderr, "MAIN: Built %s\n", il_build_date);

    void il_load_ilgraphics();
    void il_configure_ilgraphics(il_modopts *);

    il_modopts *graphics_opts = il_opts_lookup(&opts, "ilgraphics");
    if (graphics_opts) {
        il_configure_ilgraphics(graphics_opts);
    }
    il_load_ilgraphics();

    demo_start();

    void ilG_quit();
    ilG_quit();

    return 0;
}
