#include "loader.h"

#ifdef WIN32
# include <windows.h>
# include <tchar.h> 
#else
# include <dlfcn.h>
#endif
#include <stdio.h> // we don't have libilutil, so we can't use the traditional logging facilities if something goes wrong
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include "util/uthash.h" // implemented in preprocessor so it's fine
#include "util/array.h" // ditto

#if defined __APPLE__
#define SUFFIX ".dylib"
#elif defined __WIN32
#define SUFFIX ".dll"
#else
#define SUFFIX ".so"
#endif

#ifdef WIN32
typedef HINSTANCE il_handle;
#else
typedef void *il_handle;
#endif

char *strdup(const char*);

static struct module {
    char *name;
    il_handle handle;
    UT_hash_handle hh;
} *il_loaded = NULL;

static IL_ARRAY(char*,) modpaths;
static IL_ARRAY(char*,) ignores;

void il_ignore_module(const char *name)
{
    IL_APPEND(ignores, strdup(name));
}

static int check_ignored(const char *name)
{
    unsigned i;
    for (i = 0; i < ignores.length; i++) {
        if (strcmp(name, ignores.data[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void il_add_module_path(const char *path)
{
    IL_APPEND(modpaths, strdup(path));
}

char *il_normalise_module(const char *name)
{
    char *p = NULL, *sname;
    // strip path and extension off of name
#ifdef WIN32
    p = strrchr(name, '\\');
    char *p2 = strrchr(name, '/'); // path component
    if (p2 > p) {
        p = p2;
    }
#else
    p = strrchr(name, '/');
#endif
    if (p) {
        p++; // leave out the / itself
        if (strncmp(p, "lib", 3) == 0) { // lib- prefix
            sname = strdup(p + 3);
        } else {
            sname = strdup(p);
        }
    } else {
        sname = strdup(name);
    }
    if ((p = strchr(sname, '.'))) { // extension
        *p = 0;
    }
    return sname;
}

void il_load_module_dir(const char *path, il_opts *opts)
{
#ifdef WIN32
    WIN32_FIND_DATA ffd;
    HANDLE hFind;

    char dir[MAX_PATH];
    strcpy(dir, path);
    strcat(dir, "\\*");

    hFind = FindFirstFile(TEXT(dir), &ffd);

    if (INVALID_HANDLE_VALUE == hFind) {
        fprintf(stderr, "*** Failed to list directory \"%s\"\n", path);
        return;
    }

    do {
        char name[MAX_PATH];
#ifdef UNICODE
        wcstombs(name, ffd.cFileName, MAX_PATH);
#else
        strcpy(name, ffd.cFileName);
#endif
        if (strcmp(name + strlen(name) - strlen(SUFFIX), SUFFIX) != 0) {
            // assume it's not a shared library if it doesn't end with SUFFIX
            continue;
        }
        if (check_ignored(name)) {
            continue;
        }
        char buf[MAX_PATH];
        strcpy(buf, path);
        strcat(buf, "\\");
        strcat(buf, name);
        il_load_module(buf, opts);
    } while (FindNextFile(hFind, &ffd) != 0);

    FindClose(hFind);
#else
    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "*** Failed to open module directory \"%s\": %s\n", path, strerror(errno));
        return;
    }
    struct dirent entry, *result;
    while (!readdir_r(dir, &entry, &result) && result) {
        if (strcmp(result->d_name + strlen(result->d_name) - strlen(SUFFIX), SUFFIX) != 0) {
            // assume it's not a shared library if it doesn't end with SUFFIX
            continue;
        }
        if (check_ignored(result->d_name)) {
            continue;
        }
        char buf[512];
        snprintf(buf, 512, "%s/%s", path, result->d_name);
        il_load_module(buf, opts);
    }
    closedir(dir);
#endif
}

void il_load_module_paths(il_opts *opts)
{
    size_t i;
    for (i = 0; i < modpaths.length; i++) {
        il_load_module_dir(modpaths.data[i], opts);
    }
}

static char *lookup(const char *modpath, const char *name)
{
    char *str;
    if (modpath) {
        str = calloc(strlen(modpath) + 4 + strlen(name) + strlen(SUFFIX) + 1, 1);
        sprintf(str, "%s/lib%s" SUFFIX, modpath, name);
    } else {
        str = strdup(name);
    }
    if (!access(str, F_OK)) {
        return str;
    }
    free(str);
    return NULL;
}

static il_func get_symbol(struct module *mod, const char *name)
{
    il_func sym;
#ifdef WIN32
    sym = GetProcAddress(mod->handle, name);
#else
    sym = dlsym(mod->handle, name);
#endif
    return sym;
}

static const char *get_error()
{
#ifdef WIN32
    return GetLastError();
#else
    return dlerror();
#endif
}

int il_load_module(const char *name, il_opts *opts)
{
    int res;
    size_t i;
    char *sname, *path = NULL;
    struct module *mod;

    sname = il_normalise_module(name);

    HASH_FIND_STR(il_loaded, sname, mod); // look it up to see if it's already loaded
    if (mod) {
        return 0;
    }
    
    mod = calloc(1, sizeof(struct module));
    mod->name = sname;

    // look through our search paths for the module
    fprintf(stderr, "*** Loading module %s\n", sname);
    path = lookup(NULL, name);
    for (i = 0; i < modpaths.length && !path; i++) {
        path = lookup(modpaths.data[i], sname);
    }
    if (!path) {
        fprintf(stderr, "*** Could not find module %s\n", sname);
        goto fail;
    }
    char preload_name[64], load_name[64], config_name[64];
    sprintf(preload_name, "il_preload_%s", sname);
    sprintf(load_name, "il_load_%s", sname);
    sprintf(config_name, "il_configure_%s", sname);
#ifdef WIN32
    mod->handle = LoadLibrary(TEXT(path));
    if (!mod->handle) {
        fprintf(stderr, "*** Failed to load module %s: %s\n", path, get_error());
        goto fail;
    }
#else
    dlerror();
    mod->handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
    if (!mod->handle) {
        fprintf(stderr, "*** Failed to load module %s: %s\n", path, get_error());
        goto fail;
    }
#endif

    il_preload_fn deps = (il_preload_fn)get_symbol(mod, preload_name);
    if (deps) {
        const char **mods = deps();
        for (i = 0; mods[i]; i++) {
            if (il_load_module(mods[i], opts)) {
                fprintf(stderr, "*** Failed to load module %s: Dependency %s failed to load\n", path, mods[i]);
                goto fail;
            }
        }
    }

    il_configure_fn config = (il_configure_fn)get_symbol(mod, config_name);
    if (config) {
        il_modopts *modopts = il_opts_lookup(opts, il_optslice_s(sname));
        if (modopts) {
            config(modopts);
        }
    }

    il_load_fn load = (il_load_fn)get_symbol(mod, load_name);
    if (!load) {
        fprintf(stderr, "*** Failed to load %s: %s\n", sname, get_error());
        goto fail;
    }
    res = load();

    // register it as loaded
    HASH_ADD_KEYPTR(hh, il_loaded, sname, strlen(sname), mod);
    //free(sname); // memory owned by hash table entry
    free(path);
    return res;

fail:
    //free(sname);
    free(path);
    return 1;
}

static int postload(struct module *mod)
{
    char postload_name[64];
    sprintf(postload_name, "il_postload_%s", mod->name);
    il_postload_fn func = (il_postload_fn)get_symbol(mod, postload_name);
    if (!func) {
        return 1;
    }
    func();
    return 0;
}

void il_postload(const char *module)
{
    struct module *mod;
    HASH_FIND_STR(il_loaded, module, mod);
    if (!mod) {
        fprintf(stderr, "*** Could not postload: No loaded module %s\n", module);
        return;
    }
    if (!postload(mod)) {
        fprintf(stderr, "*** No postload function in %s\n", module);
        return;
    }
}

void il_postload_all()
{
    struct module *mod;
    for (mod = il_loaded; mod; mod = mod->hh.next) {
        postload(mod);
    }
}

il_func il_get_symbol(const char *module, const char *name)
{
    struct module *mod;
    HASH_FIND_STR(il_loaded, module, mod);
    if (!mod) {
        fprintf(stderr, "*** Could not look up symbol %s: No loaded module %s\n", name, module);
        return NULL;
    }
    return get_symbol(mod, name);
}

void il_close_module(const char *module)
{
    struct module *mod;
    HASH_FIND_STR(il_loaded, module, mod);
    if (!mod) {
        fprintf(stderr, "*** No such module %s\n", module);
        return;
    }
    HASH_DEL(il_loaded, mod);
#ifdef WIN32
    FreeLibrary(mod->handle);
#else
    dlclose(mod->handle);
#endif
    free(mod->name);
}

const char *il_module_iterate(void **saveptr)
{
    if (*saveptr) {
        struct module *ptr = ((struct module *)*saveptr)->hh.next;
        *saveptr = ptr;
        if (ptr) {
            return ptr->name;
        } else {
            return NULL;
        }
    }
    *saveptr = il_loaded;
    if (!il_loaded) {
        return NULL;
    }
    return il_loaded->name;
}

