#include "asset/asset.h"

#include <string.h>

#include "script/script.h"
#include "script/il.h"

int il_Asset_wrap(lua_State* L, il_Asset_Asset* asset) {
  return il_Script_createMakeLight(L, asset, "asset");
}

static int asset_index(lua_State* L) {
  il_Asset_Asset* self = il_Script_getPointer(L, 1, "asset", NULL);
  il_String key = il_Script_getString(L, 2);
  
  if (il_strcmp(key, il_l("path")) == 0) {
    il_String res = il_Asset_getPath(self);
    lua_pushlstring(L, res.data, res.length);
    return 1;
  }
  if (il_strcmp(key, il_l("data")) == 0) {
    il_String res = il_Asset_readContents(self);
    lua_pushlstring(L, res.data, res.length);
    return 1;
  }
  
  return 0;
}

static int setwritedir(lua_State* L) {
  il_String path = il_Script_getString(L, 1);
  il_Asset_setWriteDir(path);
  return 0;
}

static int registerreaddir(lua_State* L) {
  il_String path = il_Script_getString(L, 1);
  int priority = luaL_optinteger(L, 2, 0);
  il_Asset_registerReadDir(path, priority);
  return 0;
}

static int asset_close(lua_State* L) {
  il_Asset_Asset* self = il_Script_getPointer(L, 1, "asset", NULL);
  il_Asset_close(self);
  return 0;
}

static int asset_delete(lua_State* L) {
  il_Asset_Asset* self = il_Script_getPointer(L, 1, "asset", NULL);
  il_Asset_delete(self);
  return 0;
}

static int create(lua_State* L) {
  il_String path = il_Script_getString(L, 1);
  il_Asset_Asset* asset = il_Asset_open(path);
  return il_Asset_wrap(L, asset);
}

void il_Asset_luaGlobals(il_Script_Script* self, void * ctx) {
  (void)ctx;
  
  luaL_Reg l[] = {
    {"create",          &create               },
    {"getType",         &il_Script_typeGetter },
    {"isA",             &il_Script_isA        },
    
    {"setWriteDir",     &setwritedir          },
    {"registerReadDir", &registerreaddir      },
    {"close",           &asset_close          },
    {"delete",          &asset_delete         },
    
    {NULL,              NULL                  }
  };
  
  il_Script_startTable(self, l);
  
  il_Script_startMetatable(self, "asset");
  il_Script_pushFunc(self->L, "__index", &asset_index);
  
  il_Script_typeTable(self->L, "asset");
  
  il_Script_endTable(self, l, "asset");
}
