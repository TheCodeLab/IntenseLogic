#ifndef IL_LOADER_H
#define IL_LOADER_H

struct il_config; // TODO: config file structure

#ifdef WIN32
#include <windows.h>
typedef FARPROC WINAPI il_func;
#else
typedef void *il_func;
#endif

void il_modpath(const char *path);
void il_loaddir(const char *path, int argc, char **argv);
void il_loadall(int argc, char **argv);
int /*success*/ il_loadmod(const char *module, int argc, char **argv);
il_func il_getsym(const char *module, const char *name);
void il_rmmod(const char *module);

typedef int /*success*/ (*il_bootstrap_fn)(int argc, char **argv);
typedef const char **(*il_dependencies_fn)(int argc, char **argv);

#endif

