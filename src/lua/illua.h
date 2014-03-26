/** @file script.h
 * @brief Loads and runs Lua scripts
 */

#ifndef IL_SCRIPT_H
#define IL_SCRIPT_H

#include <stdlib.h>
#include <stdio.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct ilS_script {
  char *filename;
  int running;
  lua_State * L;
  int ehandler;
  size_t errlen;
  char * err;
} ilS_script;

/** Convienience macro for running a script */
#define ilS_loadfile(f) {   \
  ilS_script* script;       \
  script = ilS_new();       \
  ilS_fromFile(script, f);  \
  int res = ilS_run(script);\
  if (res != 0) {                 \
    printf("%s", script->err);    \
    abort();                      \
  }                               \
}

/** Creates a new script */
ilS_script * ilS_new();
/** Destroys a script */
void ilS_free(ilS_script* self);
/** Adds a path to package.path */
int ilS_addPath(ilS_script* self, const char *path);
/** Loads the script source */
int ilS_fromSource(ilS_script* self, const char *source, size_t len);
/** Loads script source from a file */
int ilS_fromFile(ilS_script* self, const char * filename);
/** Executes the script */
int ilS_run(ilS_script* self);

#endif
