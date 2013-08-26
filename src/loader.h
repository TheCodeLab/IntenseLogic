/** @file loader.h
 * @brief Responsible for loading modules.
 */

#ifndef IL_LOADER_H
#define IL_LOADER_H

#ifdef WIN32
#   include <windows.h>
    typedef FARPROC WINAPI il_func;
#else
    /** A generic function pointer. */
    typedef void *il_func;
#endif

/** A pointer to the symbol used to bootstrap a module.
 * @return Whether the module was bootstrapped correctly. Zero indicates
 * success.
 */
typedef int (*il_bootstrap_fn)(int argc, char **argv);

/** A pointer to the symbol used to indicate dependencies of a module.
 * @return An array of dependencies.
 */
typedef const char** (*il_dependencies_fn)(int argc, char **argv);

/** Adds a path to be searched when loading modules.
 * @param path The path to add to the list of module paths.
 */
void il_add_module_path(const char *path);

/** Loads a module by name.
 * @param module The name of the module to load.
 * @param argc The argument count passed to main.
 * @param argv The arguments passed to main.
 * @return Whether the module was loaded. Zero indicates failure.
 */
int il_load_module(const char *module, int argc, char **argv);

/** Loads all the modules in a directory.
 * @param path The path of the directory to load.
 * @param argc The argument count passed to main.
 * @param argv The arguments passed to main.
 */
void il_load_module_dir(const char *path, int argc, char **argv);

/** Loads all the modules in the paths added by il_add_module_path().
 * @param argc The argument count passed to main.
 * @param argv The arguments passed to main.
 */
void il_load_module_paths(int argc, char **argv);

/** Closes a loaded module.
 * @param module The name of the module to close.
 */
void il_close_module(const char *module);

/** Gets a symbol defined in a module.
 * @param module The name of the module to search in.
 * @param name The name of the symbol to get.
 * @return The address of the symbol.
 */
il_func il_get_symbol(const char *module, const char *name);

/** Iterates through a module. Call until it returns NULL.
 * @param saveptr A pointer to store the iterator in. Initialize to NULL.
 * @return The next module.
 */
const char *il_module_iterate(void **saveptr);

#endif

