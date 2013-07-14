#ifndef IL_LOADER_H
#define IL_LOADER_H

#ifdef WIN32
#   include <windows.h>
    typedef FARPROC WINAPI il_func;
#else
    typedef void *il_func;
#endif

typedef int (*il_bootstrap_fn)(int argc, char **argv);
typedef const char** (*il_dependencies_fn)(int argc, char **argv);

void il_add_module_path(const char *path);

int il_load_module(const char *module, int argc, char **argv);
void il_load_module_dir(const char *path, int argc, char **argv);
void il_load_module_paths(int argc, char **argv);

void il_close_module(const char *module);
il_func il_get_symbol(const char *module, const char *name);

#endif

