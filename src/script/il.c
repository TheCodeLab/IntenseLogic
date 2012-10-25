#include "il.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int il_Script_startMetatable(il_Script_Script* self, const char * name) {
  luaL_newmetatable(self->L, name);
  return 0;
}

int il_Script_endTable(il_Script_Script* self, const luaL_Reg* l, const char * name) {
  luaL_setfuncs(self->L, l, 2);
  lua_setglobal(self->L, name);
  return 0;
}

int il_Script_typeGetter(lua_State* L) {
  lua_pushinteger(L, 1);
  lua_gettable(L, lua_upvalueindex(2));
  return 1;
}

int il_Script_isA(lua_State* L) {
  //int nargs = lua_tointeger(L, lua_upvalueindex(1));
  int nargs = lua_rawlen(L, lua_upvalueindex(2));
  
  if (!lua_isstring(L, 1))
    return luaL_argerror(L, 1, "String exptected");
  
  int i;
  for (i = 1; i <= nargs; i++) {
    lua_pushinteger(L, i);
    lua_gettable(L, 2);
    if (lua_compare(L, 1, -1, LUA_OPEQ)) {
      lua_pushboolean(L, 1);
      return 1;
    }
  }
  lua_pushboolean(L,0);
  return 1;
}

int il_Script_createMakeLight(lua_State* L, void * ptr, const char * type) {
  // create userdata
  void * block = lua_newuserdata(L, sizeof(void*) + sizeof(int));
  int* is_ptr = block;
  *is_ptr = 1;
  block = is_ptr+1;
  *(void**)block = ptr;
  
  // create metatable
  luaL_getmetatable(L, type);
  lua_setmetatable(L, -2);
  
  return 1;
}

int il_Script_createMakeHeavy(lua_State* L, size_t size, const void * ptr, const char * type) {
  // create userdata
  void * block = lua_newuserdata(L, size + sizeof(int));
  int* is_ptr = block;
  *is_ptr = 0;
  block = is_ptr+1;
  memcpy(&block, ptr, size);
  
  // create metatable
  luaL_getmetatable(L, type);
  lua_setmetatable(L, -2);
  
  return 1;
}

int il_Script_pushFunc(lua_State* L, const char * name, lua_CFunction func) {
  lua_pushcfunction(L, func);
  lua_setfield(L, -2, name);
  return 0;
}

int il_Script_typeTable(lua_State* L, const char * str, ...) {
  const char * arg = str;
  va_list va;
  int i = 0;
    
  lua_newtable(L);
  va_start(va, str);
  while (arg) {
    i++;
    lua_pushinteger(L, i);
    lua_pushlstring(L, str, strlen(str));
    lua_settable(L, -3);
    arg = va_arg(va, const char*);
  }
  va_end(va);
  
  return 0;
}

const char * il_Script_getType(lua_State* L, int idx) {
  int type = lua_type(L, idx);
  if (type == LUA_TUSERDATA) {
    // TODO: somehow retrieve metatable name
  }
  return lua_typename(L, type);
}

void* il_Script_getPointer(lua_State* L, int idx, const char * type, size_t *size) {
  #if LUA_VERSION_NUM < 502
  #define lua_rawlen lua_objlen
  #endif
  
  int * is_ptr = luaL_checkudata(L, idx, type);
  if (size) *size = lua_rawlen(L, idx);
  
  if (*is_ptr) return *(void**)(is_ptr+1);
  return is_ptr+1;
}

char *strndup(const char *s, size_t n);

il_Common_String il_Script_getString(lua_State* L, int idx) {
  il_Common_String s;
  s.data = (char*)luaL_checklstring(L, idx, (size_t*)&s.length);
  s.data = strndup(s.data, s.length);
  return s;
}

double il_Script_getNumber(lua_State* L, int idx) {
  return luaL_checknumber(L, idx);
}

il_Common_String il_Script_toString(lua_State* L, int idx) {
  il_Common_String res;
  switch (lua_type(L, idx)) {
    case LUA_TNIL:
      return il_l("nil");
    case LUA_TNUMBER:
    case LUA_TSTRING:
      res.data = lua_tolstring(L, idx, &res.length);
      return res;
    case LUA_TBOOLEAN:
      if (lua_toboolean(L, idx))
        return il_l("true");
      else
        return il_l("false");
    case LUA_TTHREAD:
    case LUA_TLIGHTUSERDATA:
    case LUA_TTABLE:
    case LUA_TUSERDATA:
    case LUA_TFUNCTION: {
      char * str = malloc(sizeof(void*)*2 + 3);
      sprintf(str, "%p", lua_topointer(L, idx));
      const char * t = lua_typename(L, lua_type(L, idx));
      return il_concat(
        il_CtoS(t, -1), 
        il_l(": "), 
        il_CtoS(str, -1)
      );
    }
    case LUA_TNONE:
    default:
      return (il_Common_String){0, NULL};
  }
}

void il_Script_printStack(lua_State *L, const char* Str) {
  int J, Top;
  printf("%-26s [", Str);
  Top = lua_gettop(L);
  for (J = 1; J <= Top; J++){
    printf("%s, ", il_StoC(il_Script_toString(L, J)));
  }
  printf("]\n");
}

static struct lua_node {
  il_Script_LuaRegisterFunc func;
  void * ctx;
  struct lua_node *next;
} *lua_first = NULL;

void il_Script_registerLuaRegister(il_Script_LuaRegisterFunc func, void * ctx) {
  struct lua_node* n = calloc(1, sizeof(struct lua_node));
  n->func = func;
  n->ctx = ctx;
  n->next = NULL;
  
  if (!lua_first) {
    lua_first = n;
    return;
  }
  
  struct lua_node *last = lua_first;
  while (last->next) last = last->next;
  
  last->next = n;
}

void il_Script_openLibs(il_Script_Script* self) {
  struct lua_node * cur = lua_first;
  while (cur) {
    cur->func(self, cur->ctx);
    cur = cur->next;
  }
}
