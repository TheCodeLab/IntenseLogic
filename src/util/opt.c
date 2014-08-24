#include "opt.h"

#include <string.h>

static void do_opt(il_opts *opts, il_string modname, il_opt opt)
{
    for (unsigned i = 0; i < opts->opts.length; i++) {
        il_modopts *modopts = &opts->opts.data[i];
        if (modopts->modname.len == modname.len && strncmp(modopts->modname.str, modname.str, modname.len) == 0) {
            IL_APPEND(modopts->args, opt);
            return;
        }
    }
    il_modopts modopts;
    memset(&modopts, 0, sizeof(il_modopts));
    modopts.modname = modname;
    IL_APPEND(modopts.args, opt);
    IL_APPEND(opts->opts, modopts);
}

il_modopts *il_opts_lookup(il_opts *self, char *name)
{
    for (unsigned i = 0; i < self->opts.length; i++) {
        if (il_string_cmp(il_string_new(name), self->opts.data[i].modname)) {
            return &self->opts.data[i];
        }
    }
    return NULL;
}

il_opts il_opt_parse(int argc, char **argv)
{
    il_opts opts;
    memset(&opts, 0, sizeof(il_opts));
    for (int i = 1; i < argc; i++) {\
        if (argv[i][0] != '-') {
            il_string s = il_string_new(argv[i]);
            IL_APPEND(opts.args, s);
            continue;
        }
        il_opt opt;
        char *dot, *start = argv[i] + 1, *equals = strchr(start, '='),  *namestart = start, *nameend = argv[i] + strlen(argv[i]);
        il_string modname = {NULL, 0};
        il_string name    = {NULL, 0};
        il_string arg     = {NULL, 0};
        if (((dot = strchr(start, '.'))) && (!equals || dot < equals)) {
            modname = il_string_bin(start, dot - start);
            namestart = dot+1;
        }
        if (equals) {
            arg = il_string_new(equals+1);
            nameend = equals;
        }
        name = il_string_bin(namestart, nameend - namestart);
        opt = (il_opt){name, arg};
        do_opt(&opts, modname, opt);
    }
    return opts;
}
