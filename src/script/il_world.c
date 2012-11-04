#include "common/world.h"
#include "graphics/world.h"

#include <string.h>

#include "script/script.h"
#include "script/il.h"
#include "interface.h"

int il_Graphics_World_wrap(lua_State* L, il_Graphics_World * w) {
  return il_Script_createMakeLight(L, w, "world");
}

int il_Common_World_wrap(lua_State* L, il_Common_World * self) {
  il_Graphics_World* w = il_Common_World_getGraphicsWorld(self);
  if (w) return il_Graphics_World_wrap(L, w);
  return il_Graphics_World_wrap(L, il_Graphics_World_new_world(self));
}

static int world_create(lua_State* L) {
  il_Graphics_World* w = il_Graphics_World_new();
  return il_Graphics_World_wrap(L, w);
}

static int world_index(lua_State* L) {
  il_Graphics_World* self = il_Script_getPointer(L, 1, "world", NULL);
  const char * k = luaL_checkstring(L, 1);
  
  if (strcmp(k, "camera")) {
    return il_Graphics_Camera_wrap(L, self->camera);
  }
  
  return 0;
}

void il_Graphics_World_luaGlobals(il_Script_Script* self, void * ctx) {
  (void)ctx;

  const luaL_Reg l[] = {
    {"create", &world_create},
    {"getType", &il_Script_typeGetter},
    {"isA", &il_Script_isA},
    
    {NULL, NULL}
  };

  il_Script_startTable(self, l);

  il_Script_startMetatable(self, "world");
  il_Script_pushFunc(self->L, "__index", &world_index);
  
  il_Script_typeTable(self->L, "world");
  
  il_Script_endTable(self, l, "world");
}
