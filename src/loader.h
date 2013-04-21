#ifndef IL_LOADER_H
#define IL_LOADER_H

struct il_config; // TODO: config file structure

int /*success*/ il_loadmod(const char *name, struct il_config *config);

typedef int /*success*/ (*il_bootstrap_fn)(struct il_config *config);

#endif

