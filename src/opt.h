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

il_opts il_opt_parse(int argc, char **argv);

