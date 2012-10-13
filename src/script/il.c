#include "il.h"

#include <stdarg.h>
#include <stdio.h>

int il_Script_startTable(il_Script_Script* self) {
  lua_newtable(self->L);
  return 0;
}

int il_Script_addFunc(il_Script_Script* self, const char * name, lua_CFunction func) {
  lua_pushcfunction(self->L, func);
  lua_setfield(self->L, -2, name);
  return 0;
}

int il_Script_addClosure(il_Script_Script* self, const char * name, lua_CFunction func, int n) {
  lua_pushcclosure(self->L, func, n);
  lua_setfield(self->L, -2, name);
  return 0;
}

static int getter(lua_State* L) {
  size_t len;
  const char * str = lua_tolstring(L, lua_upvalueindex(1), &len);
  lua_pushlstring(L, str, len);
  
  return 1;
}
int il_Script_addTypeGetter(il_Script_Script* self, const char * name) {
  lua_pushlstring(self->L, name, strlen(name));
  return il_Script_addClosure(self, "getType", &getter, 1);
}

static int isa(lua_State* L) {
  int nargs = lua_tointeger(L, lua_upvalueindex(1));
      
  if (!lua_isstring(L, 1))
    return luaL_argerror(L, 1, "String exptected");
  
  int i;
  for (i = 1; i <= nargs; i++) {
    if (lua_equal(L, 1, lua_upvalueindex(i + 1))) {
      lua_pushboolean(L, 1);
      return 1;
    }
  }
  lua_pushboolean(L,0);
  return 1;
}
int il_Script_addIsAfunc(il_Script_Script* self, const char * arg1, ...) {
  int nargs = 0;
  va_list ap;
  
  lua_pushnil(self->L);
  int pos = lua_gettop(self->L);
  va_start(ap, arg1);
  const char * cur = arg1;
  for (cur = arg1; cur; cur = va_arg(ap, const char*)) {
    lua_pushlstring(self->L, cur, strlen(cur));
    nargs++;
  }
  va_end(ap);
  lua_pushinteger(self->L, nargs);
  lua_replace(self->L, pos);
  
  return il_Script_addClosure(self, "isA", &isa, nargs+1);
}

int il_Script_endTable(il_Script_Script* self, const char * name, lua_CFunction call) {
  lua_setglobal(self->L, name);
  
  lua_newtable(self->L);
  lua_pushcfunction(self->L, call);
  lua_setfield(self->L, -2, "__call");
  lua_setmetatable(self->L, -2);
  return 0;
}

int il_Script_createHelper(lua_State* L, void * ptr, const char * type) {
   // create table
  lua_newtable(L);
  lua_pushlightuserdata(L, ptr);
  lua_setfield(L, -2, "ptr");
  lua_pushlstring(L, type, strlen(type));
  lua_setfield(L, -2, "type");
    
  // create metatable
  lua_newtable(L);
  lua_getglobal(L, type);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);
  
  return 1;
}

void* il_Script_getPointer(lua_State* L, int idx, const char * type) {
  char * msg = calloc(1, strlen(type) + 10);
  strcpy(msg, "Expected ");
  strcat(msg, type);
  //printf("is table\n");
  if (!lua_istable(L, idx))
    luaL_argerror(L, idx, msg);
  
  lua_pushliteral(L, "type");
  lua_rawget(L, idx);
  lua_pushlstring(L, type, strlen(type));
  //printf("type equal\n");
  if (!lua_equal(L, -1, -2))
    luaL_argerror(L, 1, msg);
  lua_pop(L, 2);
  
  lua_pushliteral(L, "ptr");
  lua_rawget(L, 1);
  //printf("is light userdata\n");
  if (!lua_islightuserdata(L, -1))
    luaL_argerror(L, 1, msg);
  
  return lua_touserdata(L, -1);
}

il_Common_String il_Script_getString(lua_State* L, int idx) {
  il_Common_String s;
  if (!lua_isstring(L, idx)) {
    luaL_argerror(L, idx, "Expected string");
    return (il_Common_String){0, NULL};
  }
  s.data = (char*)lua_tolstring(L, idx, (size_t*)&s.length);
  return s;
}
