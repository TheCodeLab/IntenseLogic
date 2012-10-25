#include "common/input.h"

#include "common/keymap.h"
#include "script/script.h"
#include "script/il.h"

static int keydown(lua_State* L) {
  int key;
  if (lua_isstring(L, 1))
    key = il_Common_Keymap_getkey(lua_tostring(L, 1));
  else if (lua_isnumber(L, 1))
    key = (int)luaL_checkinteger(L, 1);
  else luaL_argerror(L, 1, "Expected string or number");
  
  int res = il_Input_isKeySet(key);
  lua_pushboolean(L, res);
  return 1;
}

static int mousedown(lua_State* L) {
  int key;
  if (lua_isstring(L, 1))
    key = il_Common_Keymap_getkey(lua_tostring(L, 1));
  else if (lua_isnumber(L, 1))
    key = (int)luaL_checkinteger(L, 1);
  else luaL_argerror(L, 1, "Expected string or number");
  
  int res = il_Input_isButtonSet(key);
  lua_pushboolean(L, res);
  return 1;
}

static int grabmouse(lua_State* L) {
  int b = lua_toboolean(L, 1);
  
  il_Input_GrabMouse(b);
  return 0;
}

void il_Input_luaGlobals(il_Script_Script* self, void * ctx) {

  const luaL_Reg l[] = {
    // this is not a typical object, as it does not have instances.
    
    {"keyDown",   &keydown  },
    {"mouseDown", &mousedown},
    {"grabMouse", &grabmouse},
    
    {NULL,        NULL      }
  };

  il_Script_startTable(self, l);

  il_Script_startMetatable(self, "input");
  
  il_Script_typeTable(self->L, "input");
  
  il_Script_endTable(self, l, "input");
}
