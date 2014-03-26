#ifndef IL_OPT_H
#define IL_OPT_H

#include <stdbool.h>

#include "util/array.h"

typedef struct il_optslice {
    const char *str;
    size_t len;
} il_optslice;

typedef struct il_opt {
    il_optslice name, arg;
} il_opt;

typedef struct il_modopts {
    il_optslice modname;
    IL_ARRAY(il_opt,) args;
} il_modopts;

typedef struct il_opts {
    IL_ARRAY(il_modopts,) opts;
    IL_ARRAY(il_optslice,) args;
} il_opts;

#define il_optslice_s(s) (il_optslice){s, strlen(s)}

bool il_opts_cmp(il_optslice a, il_optslice b);
il_modopts *il_opts_lookup(il_opts *self, il_optslice name);
il_opts il_opt_parse(int argc, char **argv);

#endif

