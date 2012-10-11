#include "script.h"

#include <stdio.h>

void il_Script_init(){

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
  
  int res = lua_load(self->L, &reader, &data, chunkname);
  if (res) {
    self->err = lua_tolstring(self->L, -1, &self->errlen);
    self->running = -1;
    return -1;
  }
  return 0;
}

int il_Script_fromFile(il_Script_Script* self, const char * filename) {
  return il_Script_fromAsset(self, il_Asset_open(il_Common_fromC((char*)filename)));
}

int il_Script_run(il_Script_Script* self) {
  self->running = 1;
  int res = lua_pcall(self->L, 0, 0, 0);
  if (res) {
    self->err = lua_tolstring(self->L, -1, &self->errlen);
    self->running = -1;
    return -1;
  }
  return 0;
}
