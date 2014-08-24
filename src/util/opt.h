#ifndef IL_OPT_H
#define IL_OPT_H

#include <stdbool.h>

#include "util/array.h"
#include "util/ilstring.h"

typedef struct il_opt {
    il_string name, arg;
} il_opt;

typedef struct il_modopts {
    il_string modname;
    IL_ARRAY(il_opt,) args;
} il_modopts;

typedef struct il_opts {
    IL_ARRAY(il_modopts,) opts;
    IL_ARRAY(il_string,) args;
} il_opts;

il_modopts *il_opts_lookup(il_opts *self, char *name);
il_opts il_opt_parse(int argc, char **argv);

#endif
