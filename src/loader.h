#ifndef IL_LOADER_H
#define IL_LOADER_H

struct il_config; // TODO: config file structure

int /*success*/ il_loadmod(const char *module, int argc, char **argv);
void *il_getsym(const char *module, const char *name);
void il_rmmod(const char *module);

typedef int /*success*/ (*il_bootstrap_fn)(int argc, char **argv);

#endif

