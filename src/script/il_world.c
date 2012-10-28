#include "common/world.h"

#include "script/script.h"
#include "script/il.h"

int il_Common_World_wrap(lua_State* L, il_Common_World * w) {
  return il_Script_createMakeLight(L, w, "world");
}

static int world_create(lua_State* L) {
  il_Common_World* w = il_Common_World_new();
  return il_Common_World_wrap(L, w);
}

void il_Common_World_luaGlobals(il_Script_Script* self, void * ctx) {
  (void)ctx;

  const luaL_Reg l[] = {
    {"create", &world_create},
    {"getType", &il_Script_typeGetter},
    {"isA", &il_Script_isA},
    
    {NULL, NULL}
  };

  il_Script_startTable(self, l);

  il_Script_startMetatable(self, "world");
  
  il_Script_typeTable(self->L, "world");
  
  il_Script_endTable(self, l, "world");
}
