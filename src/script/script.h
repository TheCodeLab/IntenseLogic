#ifndef IL_SCRIPT_H
#define IL_SCRIPT_H

#include <stdlib.h>
#include <stdio.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "common/string.h"
#include "asset/asset.h"

void il_Script_init();

typedef struct il_Script_Script {
  il_Common_String source;
  const char * filename;
  int running;
  lua_State * L;
  int ehandler;
  size_t errlen;
  const char * err;
} il_Script_Script;

#define il_Script_loadfile(f) {   \
  il_Script_Script* script;       \
  script = il_Script_new();       \
  il_Script_fromFile(script, f);  \
  int res = il_Script_run(script);\
  if (res != 0) {                 \
    abort();                      \
  }                               \
}

il_Script_Script * il_Script_new();
int il_Script_fromAsset(il_Script_Script*, il_Asset_Asset * asset);
int il_Script_fromSource(il_Script_Script*, il_Common_String source);
int il_Script_fromFile(il_Script_Script*, const char * filename);
int il_Script_run(il_Script_Script*);

#endif
