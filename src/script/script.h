#ifndef IL_SCRIPT_H
#define IL_SCRIPT_H

#include <stdlib.h>
#include <stdio.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "common/string.h"
#include "asset/asset.h"

void ilS_init();

typedef struct ilS_script {
  il_string source;
  const char * filename;
  int running;
  lua_State * L;
  int ehandler;
  size_t errlen;
  const char * err;
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
int ilS_fromAsset(ilS_script*, ilA_asset * asset);
int ilS_fromSource(ilS_script*, il_string source);
int ilS_fromFile(ilS_script*, const char * filename);
int ilS_run(ilS_script*);

#endif
