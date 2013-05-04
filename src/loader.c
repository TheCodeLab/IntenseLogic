#include "loader.h"

#include <dlfcn.h>
#include <stdio.h> // we don't have libilutil, so we can't use the traditional logging facilities if something goes wrong
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include "util/uthash.h" // implemented in preprocessor so it's fine
#include "util/array.h" // ditto

static struct module {
    char *name;
    void *handle;
    UT_hash_handle hh;
} *il_loaded = NULL;

static IL_ARRAY(char*,) modpaths;

void il_modpath(const char *path)
{
    IL_APPEND(modpaths, strdup(path));
}

void il_loaddir(const char *path, int argc, char **argv)
{
    // TODO: windows
    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "Failed to open modules directory: %s\n", strerror(errno));
    }
    struct dirent entry, *result;
    while (!readdir_r(dir, &entry, &result) && result) {
        if (strcmp(result->d_name + strlen(result->d_name) - 3, ".so") != 0) {
            // assume it's not a shared library if it doesn't end with .so
            continue;
        }
        char buf[512];
        snprintf(buf, 512, "%s/%s", path, result->d_name);
        il_loadmod(buf, argc, argv);
    }
    closedir(dir);
}

void il_loadall(int argc, char **argv)
{
    int i;
    for (i = 0; i < modpaths.length; i++) {
        il_loaddir(modpaths.data[i], argc, argv);
    }
}

static char *lookup(const char *modpath, const char *name)
{
    // TODO: windows
    char *str;
    if (modpath) {
        str = calloc(strlen(modpath) + 4 + strlen(name) + 4, 1);
        sprintf(str, "%s/lib%s.so", modpath, name);
    } else {
        str = strdup(name);
    }
    if (!access(str, F_OK)) {
        return str;
    }
    free(str);
    return NULL;
}

int il_loadmod(const char *name, int argc, char **argv)
{
    int stripped = 0, res, i;
    char *sname, *path = NULL, *p;
    struct module *mod;
    void *handle;
    const char *error;

    // strip path and extension off of name
    p = strrchr(name, '/'); // path component
    if (p) {
        p++; // leave out the / itself
        stripped = 1;
        if (strncmp(p, "lib", 3) == 0) { // lib- prefix
            sname = strdup(p + 3);
        } else {
            sname = strdup(p);
        }
    } else {
        sname = strdup(name);
    }
    if ((p = strchr(sname, '.'))) { // extension
        stripped = 1;
        *p = 0;
    }
    HASH_FIND_STR(il_loaded, sname, mod); // look it up to see if it's already loaded
    if (mod) {
        return 1;
    }
    
    // look through our search paths for the module
    fprintf(stderr, "*** Loading module %s\n", sname);
    path = lookup(NULL, name);
    for (i = 0; i < modpaths.length && !path; i++) {
        path = lookup(modpaths.data[i], sname);
    }
    dlerror();
    if (!path) {
        fprintf(stderr, "*** Could not find module %s\n", sname);
        return 0;
    }
    handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        fprintf(stderr, "*** Failed to load module %s: %s\n", path, dlerror());
        return 0;
    }

    // load the il_bootstrap symbol
    dlerror();
    il_bootstrap_fn func = (il_bootstrap_fn)dlsym(handle, "il_bootstrap");
    error = dlerror();
    if (error) {
        fprintf(stderr, "*** Failed to load symbol: %s\n", error);
        dlclose(handle);
        return 0;
    }

    // initialize the module
    res = func(argc, argv);

    // register it as loaded
    mod = calloc(1, sizeof(struct module));
    mod->name = sname;
    mod->handle = handle;
    HASH_ADD_KEYPTR(hh, il_loaded, sname, strlen(sname), mod);
    return res;
}

void *il_getsym(const char *module, const char *name)
{
    struct module *mod;
    HASH_FIND_STR(il_loaded, module, mod);
    if (!mod) {
        fprintf(stderr, "*** No loaded module %s\n", module);
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

