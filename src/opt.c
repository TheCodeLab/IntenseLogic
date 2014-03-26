#include "opt.h"

#include <string.h>

static void do_opt(il_opts *opts, il_optslice modname, il_opt opt)
{
    for (unsigned i = 0; i < opts->opts.length; i++) {
        il_modopts *modopts = &opts->opts.data[i];
        if (modopts->modname.len == modname.len && strncmp(modopts->modname.str, modname.str, modname.len) == 0) {
            IL_APPEND(modopts->args, opt);
            return;
        }
    }
    il_modopts modopts = {};
    modopts.modname = modname;
    IL_APPEND(modopts.args, opt);
    IL_APPEND(opts->opts, modopts);
}

il_opts il_opt_parse(int argc, char **argv)
{
    il_opts opts = {};
    for (int i = 1; i < argc; i++) {\
        if (argv[i][0] != '-') {
            il_optslice s = {argv[i], strlen(argv[i])};
            IL_APPEND(opts.args, s);
            continue;
        }
        il_opt opt;
        char *equals, *dot, *start = argv[i] + 1, *namestart = start, *nameend = argv[i] + strlen(argv[i]);
        il_optslice modname = {NULL, 0},
                    name    = {NULL, 0},
                    arg     = {NULL, 0};
        if ((dot = strchr(start, '.'))) {
            modname = (il_optslice){start, dot - start};
            namestart = dot+1;
        }
        if ((equals = strchr(start, '='))) {
            il_opt opt;
            arg = (il_optslice){equals+1, strlen(equals+1)};
            nameend = equals;
        }
        name = (il_optslice){namestart, nameend - namestart};
        opt = (il_opt){name, arg};
        do_opt(&opts, modname, opt);
    }
    return opts;
}

