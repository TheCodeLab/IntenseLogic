#include "common/world.h"

#include "script/script.h"
#include "script/il.h"

static int world_create(lua_State* L) {
  il_Common_World* w = il_Common_World_new();
  
  il_Script_createMakeLight(L, w, "world");

  return il_Script_createEndMt(L);
}

void il_Common_World_luaGlobals(il_Script_Script* self, void * ctx) {
  il_Script_startTable(self);
  
  il_Script_addFunc(self, "create", &world_create);
  il_Script_addTypeGetter(self, "world");
  il_Script_addIsA(self, "world");
  
  il_Script_startMetatable(self, "world", &world_create);
  
  il_Script_endMetatable(self);
}
