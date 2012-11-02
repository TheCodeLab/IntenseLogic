#include "graphics/drawable3d.h"
#include "graphics/shape.h"
#include "graphics/terrain.h"

#include "script/script.h"
#include "script/il.h"
#include "interface.h"
#include "common/terrain.h"

int il_Graphics_Drawable3d_wrap(lua_State* L, il_Graphics_Drawable3d* self) {
  return il_Script_createMakeLight(L, self, "drawable");
}

void il_Graphics_Drawable3d_luaGlobals(il_Script_Script* self, void * ctx) {
  (void)ctx;

  const luaL_Reg l[] = {
    {NULL, NULL}
  };
  
  il_Script_startTable(self, l);
  
  il_Script_startMetatable(self, "drawable");
  
  il_Script_typeTable(self->L, "drawable");
  
  il_Script_endTable(self, l, "drawable");
  
}
