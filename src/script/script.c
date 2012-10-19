#include "script.h"

#include "script/il.h"
#include "common/log.h"
#include "script/interface.h"

int il_Script_Script_wrap(lua_State* L, il_Script_Script* s);

void il_Script_init(){
  il_Script_registerLuaRegister(&il_Script_luaGlobals, NULL);
  il_Script_registerLuaRegister(&sg_Vector_luaGlobals, NULL);
  il_Script_registerLuaRegister(&il_Common_Positionable_luaGlobals, NULL);
  il_Script_registerLuaRegister(&il_Common_World_luaGlobals, NULL);
  il_Script_registerLuaRegister(&sg_Quaternion_luaGlobals, NULL);
}

static int print(lua_State* L) {
  int nargs = lua_gettop(L);
  il_Common_String strs[nargs];
  il_Common_String str;
  int i;
  
  if (nargs < 1) {
    printf("\n");
    return 0;
  }
  
  for (i = 1; i <= nargs; i++) {
    strs[i-1] = il_Script_toString(L, i);
  }
  str = strs[0];
  for (i = 2; i <= nargs; i++) {
    if (!strs[i].length) continue;
    str = il_concat(str, il_l("\t"), strs[i-1]);
  }
  lua_Debug ar;
  int res = lua_getstack(L, 1, &ar);
  if (res != 1) return -1;
  res = lua_getinfo(L, "nSl", &ar);
  if (!res) return -1;
  
  printf( "%s:%i (%s) %s: %s\n",
    ar.short_src,
    ar.currentline,
    ar.name,
    "NOTICE",
    il_StoC(str)
  );
  return 0;
}

il_Script_Script * il_Script_new() {
  il_Script_Script* s = calloc(sizeof(il_Script_Script), 1);
  
  return s;
}

int il_Script_fromAsset(il_Script_Script* self, il_Asset_Asset * asset) {
  if (!self || !asset) return -1;
  self->filename = il_Common_toC(il_Asset_getPath(asset));
  return il_Script_fromSource(self, il_Asset_readContents(asset));
}

struct reader_ctx {
  int loaded;
  il_Common_String source;
};
static const char * reader(lua_State* L, void * data, size_t * size) {
  struct reader_ctx * ctx = (struct reader_ctx*)data;
  if (ctx->loaded) return NULL;
  *size = ctx->source.length;
  ctx->loaded = 1;
  return ctx->source.data;
}

int il_Script_fromSource(il_Script_Script* self, il_Common_String source) {
  if (!self || !source.length) return -1;
  self->source = source;
  struct reader_ctx data;
  data.loaded = 0;
  data.source = source;
  char * chunkname;
  if (self->filename) {
    chunkname = malloc(strlen(self->filename) + 2);
    sprintf(chunkname, "@%s", self->filename);
  }
  
  if (!self->L)
    self->L = luaL_newstate();
  luaL_openlibs(self->L);
  
  lua_pushcfunction(self->L, &print);
  lua_setglobal(self->L, "print");
  
  il_Script_openLibs(self);
  
  #if LUA_VERSION_NUM == 502
  int res = lua_load(self->L, &reader, &data, chunkname, NULL);
  #else
  int res = lua_load(self->L, &reader, &data, chunkname);
  #endif
  if (res) {
    self->err = lua_tolstring(self->L, -1, &self->errlen);
    self->running = -1;
    return -1;
  }
  return 0;
}
static int fromsource(lua_State* L) {
  
  il_Script_Script* self = (il_Script_Script*)il_Script_getPointer(L, 1, "script", NULL);
  il_Common_String source = il_Script_getString(L, 2);
  
  int res = il_Script_fromSource(self, source);
  if (res != 0) {
    return luaL_error(L, self->err);
  }
  return 0;
}

int il_Script_fromFile(il_Script_Script* self, const char * filename) {
  return il_Script_fromAsset(self, il_Asset_open(il_Common_fromC((char*)filename)));
}
static int fromfile(lua_State* L) {
  il_Script_Script* self = (il_Script_Script*)il_Script_getPointer(L, 1, "script", NULL);
  il_Common_String filename = il_Script_getString(L, 2);
  
  int res = il_Script_fromFile(self, filename.data);
  if (res != 0) {
    return luaL_error(L, self->err);
  }
  return 0;
}

int il_Script_run(il_Script_Script* self) {
  self->running = 1;
  il_Common_log(3, "Running script %s", self->filename);
  int res = lua_pcall(self->L, 0, 0, 0);
  if (res) {
    self->err = lua_tolstring(self->L, -1, &self->errlen);
    self->running = -1;
    return -1;
  }
  return 0;
}
static int run(lua_State* L) {
  il_Script_Script* self = (il_Script_Script*)il_Script_getPointer(L, 1, "script", NULL);
  
  int res = il_Script_run(self);
  if (res != 0) {
    return luaL_error(L, "Error running script");
  }
  return 0;
}

int il_Script_Script_wrap(lua_State* L, il_Script_Script* s) {
  il_Script_createMakeLight(L, s, "script");
  return il_Script_createEndMt(L);
}

static int create(lua_State* L) {
  il_Script_Script* self = il_Script_new();
  return il_Script_Script_wrap(L, self);
}

void il_Script_luaGlobals(il_Script_Script* self, void * ctx) {
  il_Script_startTable(self);
  
  il_Script_addFunc(self, "create", &create);
  il_Script_addTypeGetter(self, "script");
  il_Script_addIsA(self, "script");
  
  il_Script_addFunc(self, "fromSource", &fromsource);
  il_Script_addFunc(self, "fromFile", &fromfile);
  il_Script_addFunc(self, "run", &run);
  
  il_Script_startMetatable(self, "script", &create);
  il_Script_endMetatable(self);
}
