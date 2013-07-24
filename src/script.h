#ifndef IL_SCRIPT_H
#define IL_SCRIPT_H

#include <stdlib.h>
#include <stdio.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

//#include "util/ilstring.h"

typedef struct ilS_script {
  //char *source;
  //size_t source_len;
  char *filename;
  int running;
  lua_State * L;
  int ehandler;
  size_t errlen;
  char * err;
} ilS_script;

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

ilS_script * ilS_new();
void ilS_free(ilS_script* self);
int ilS_addPath(ilS_script* self, const char *path);
//int ilS_fromAsset(ilS_script* self, ilA_asset * asset);
int ilS_fromSource(ilS_script* self, const char *source, size_t len);
int ilS_fromFile(ilS_script* self, const char * filename);
int ilS_run(ilS_script* self);

#endif
