#include "loader.h"

#include <dlfcn.h>
#include <stdio.h> // we don't have libilutil, so we can't use the traditional logging facilities if something goes wrong

int il_loadmod(const char *name, int argc, char **argv)
{
    // TODO: windows
    dlerror();
    void *handle = dlopen(name, RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        fprintf(stderr, "Failed to load module %s: %s\n", name, dlerror());
        return 0;
    }
    dlerror();
    il_bootstrap_fn func = (il_bootstrap_fn)dlsym(handle, "il_bootstrap");
    const char *error = dlerror();
    if (error) {
        fprintf(stderr, "Failed to load symbol: %s\n", error);
        return 0;
    }
    int res = func(argc, argv);
    dlclose(handle);
    return res;
}
