#include <stdio.h>

#include "opt.h"

int main(int argc, char **argv)
{
    il_opts opts = il_opt_parse(argc, argv);
    printf("Args:\n");
    for (unsigned i = 0; i < opts.args.length; i++) {
        printf("\t%.*s\n", (int)opts.args.data[i].len, opts.args.data[i].str);
    }
    for (unsigned i = 0; i < opts.opts.length; i++) {
        il_modopts *mod = &opts.opts.data[i];
        printf("Module %.*s (%zu):\n", (int)mod->modname.len, mod->modname.str, mod->modname.len);
        for (unsigned j = 0; j < mod->args.length; j++) {
            il_opt *arg = &mod->args.data[j];
            printf("\t%.*s = %.*s\n", (int)arg->name.len, arg->name.str, (int)arg->arg.len, arg->arg.str);
        }
    }
}

