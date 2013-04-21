#include "loader.h"

#include <dlfcn.h>
#include <stdio.h> // we don't have libilutil, so we can't use the traditional logging facilities if something goes wrong
#include <string.h>
#include <stdlib.h>

#include "util/uthash.h" // implemented in preprocessor so it's fine

static struct module {
    char *name;
    void *handle;
    UT_hash_handle hh;
} *il_loaded = NULL;

int il_loadmod(const char *name, int argc, char **argv)
{
    fprintf(stderr, "*** Loading module %s\n", name);
    // TODO: windows
    dlerror();
    void *handle = dlopen(name, RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        fprintf(stderr, "*** Failed to load module %s: %s\n", name, dlerror());
        return 0;
    }
    dlerror();
    il_bootstrap_fn func = (il_bootstrap_fn)dlsym(handle, "il_bootstrap");
    const char *error = dlerror();
    if (error) {
        fprintf(stderr, "*** Failed to load symbol: %s\n", error);
        dlclose(handle);
        return 0;
    }
    int res = func(argc, argv);
    struct module *mod = calloc(1, sizeof(struct module));
    mod->name = strdup(strrchr(name,'/')+1); // remove path component
    char *p = strchr(mod->name, '.'); // remove extension
    if (p) {
        *p = 0;
    }
    mod->handle = handle;
    HASH_ADD_KEYPTR(hh, il_loaded, mod->name, strlen(mod->name), mod);
    return res;
}

void *il_getsym(const char *module, const char *name)
{
    struct module *mod;
    HASH_FIND_STR(il_loaded, module, mod);
    if (!mod) {
        fprintf(stderr, "*** No such module %s\n", module);
        return NULL;
    }
    void *sym = dlsym(mod->handle, name);
    const char *error = dlerror();
    if (error) {
        fprintf(stderr, "*** Failed to load symbol: %s\n", error);
        return NULL;
    }
    return sym;
}

void il_rmmod(const char *module)
{
    struct module *mod;
    HASH_FIND_STR(il_loaded, module, mod);
    if (!mod) {
        fprintf(stderr, "*** No such module %s\n", module);
        return;
    }
    HASH_DEL(il_loaded, mod);
    dlclose(mod->handle);
    free(mod->name);
}

