#include "script.h"

#include "script/il.h"
#include "common/log.h"

void sg_Vector_luaGlobals(il_Script_Script* self, void* ctx);
void il_Script_luaGlobals(il_Script_Script* self, void * ctx);

void il_Script_init(){
  il_Script_registerLuaRegister(&il_Script_luaGlobals, NULL);
  il_Script_registerLuaRegister(&sg_Vector_luaGlobals, NULL);
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
const char * reader(lua_State* L, void * data, size_t * size) {
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
  const char * chunkname = self->filename;
  
  if (!self->L)
    self->L = luaL_newstate();
  luaL_openlibs(self->L);
  
  il_Script_openLibs(self);
  
  int res = lua_load(self->L, &reader, &data, chunkname);
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

static int create(lua_State* L) {
  il_Script_Script* self = il_Script_new();
  il_Script_createMakeLight(L, self, "script");
  return il_Script_createEndMt(L);
}

void il_Script_luaGlobals(il_Script_Script* self, void * ctx) {
  il_Script_startTable(self);
  
  il_Script_addFunc(self, "create", &create);
  il_Script_addTypeGetter(self, "script");
  il_Script_addIsA(self, "Script");
  
  il_Script_addFunc(self, "fromSource", &fromsource);
  il_Script_addFunc(self, "fromFile", &fromfile);
  il_Script_addFunc(self, "run", &run);
  
  il_Script_startMetatable(self, "script", &create);
  il_Script_endMetatable(self);
}
