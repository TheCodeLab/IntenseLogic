#include "log.h"

//#include <mowgli.h>

#include "util/alloc.h"
#include "util/uthash.h"

const char *il_log_prefixes[] = {
    "fatal: ",
    "error: ",
    "warning: ",
    "",
    "debug: "
};

struct module {
    char *name;
    int level;
    UT_hash_handle hh;
} *modules = NULL;

int il_can_log(const char *file, int level)
{
    if (!modules) {
        return level <= 3;
    }
    char *s = strdup(file);
    if (strchr(s, '/')) {
        *strchr(s, '/') = 0;
    }
    struct module* mod;
    HASH_FIND_STR(modules, s, mod);
    free(s);
    if (!mod) {
        return level <= 3;
    } else {
        return level <= mod->level;
    }
}

void il_log_toggle(const char *module, int level)
{
    struct module *mod = il_alloc(NULL, sizeof(struct module));
    mod->level = level;
    mod->name = strdup(module);
    struct module *old;
    HASH_REPLACE_STR(modules, name, mod, old);
    if (old) {
        free(old->name);
        il_free(NULL, old);
    }
}

