#ifndef IL_LOADER_H
#define IL_LOADER_H

struct il_config; // TODO: config file structure

int /*success*/ il_loadmod(const char *name, int argc, char **argv);

typedef int /*success*/ (*il_bootstrap_fn)(int argc, char **argv);

#endif

