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

int il_Script_startMetatable(il_Script_Script* self, const char * name, lua_CFunction call) {
  lua_pushvalue(self->L, -1);
  lua_setglobal(self->L, name);
  
  lua_newtable(self->L);
  lua_pushcfunction(self->L, call);
  lua_setfield(self->L, -2, "__call");
  return 0;
}

int il_Script_endMetatable(il_Script_Script* self) {
  lua_setmetatable(self->L, -2);
  return 0;
}

int il_Script_createMakeLight(lua_State* L, void * ptr, const char * type) {
  // create userdata
  il_Script_TypedPointer * udata = (il_Script_TypedPointer*)lua_newuserdata(L, sizeof(il_Script_TypedPointer));
  udata->is_pointer = 1;
  udata->ptr = ptr;
  udata->type = type;
  
  // create metatable
  lua_newtable(L);
  lua_getglobal(L, type);
  lua_setfield(L, -2, "__index");
  
  return 1;
}

int il_Script_createMakeHeavy(lua_State* L, size_t size, const void * ptr, const char * type) {
  // create userdata
  il_Script_TypedBox * udata = (il_Script_TypedBox*)lua_newuserdata(L, sizeof(il_Script_TypedBox) + size);
  udata->is_pointer = 0;
  udata->type = type;
  memcpy(&udata->data, ptr, size);
  
  // create metatable
  lua_newtable(L);
  int idx = lua_gettop(L);
  lua_getglobal(L, type);
  lua_setfield(L, -2, "__index");
  
  return idx;
}

int il_Script_createEndMt(lua_State* L) {
  lua_setmetatable(L, -2);
  return 1;
}

const char * il_Script_getType(lua_State* L, int idx) {
  int type = lua_type(L, idx);
  if (type == LUA_TUSERDATA) {
    void * raw_ptr = lua_touserdata(L, idx);
    if (*(int*)raw_ptr) { // is_pointer
      il_Script_TypedPointer* ptr = (il_Script_TypedPointer*)raw_ptr;
      return ptr->type;
    } else {
      il_Script_TypedBox* ptr = (il_Script_TypedBox*)raw_ptr;
      return ptr->type;
    }
  }
  return lua_typename(L, type);
}

void* il_Script_getPointer(lua_State* L, int idx, const char * type, size_t *size) {
  char * msg = calloc(1, strlen(type) + 10);
  strcpy(msg, "Expected ");
  strcat(msg, type);
  
  if (!lua_isuserdata(L, idx) && !lua_isuserdata(L, idx))
    goto error;
  
  void *raw_ptr = lua_touserdata(L, idx);
  if (*(int*)raw_ptr) { // is_pointer
    il_Script_TypedPointer* ptr = (il_Script_TypedPointer*)raw_ptr;
    if (strcmp(ptr->type, type) != 0)
      goto error;
    
    if (size)
      *size = 0;
    
    return ptr->ptr;
  } else {
    il_Script_TypedBox* ptr = (il_Script_TypedBox*)raw_ptr;
    if (strcmp(ptr->type, type) != 0)
      goto error;
    
    if (size)
      *size = ptr->size;
    
    return &ptr->data;
  }
  
  error:
  luaL_argerror(L, idx, msg);
  return NULL;
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

double il_Script_getNumber(lua_State* L, int idx) {
  if (!lua_isnumber(L, idx)) {
    luaL_argerror(L, idx, "Expected number");
    return 0.0;
  }
  return (double)lua_tonumber(L, idx);
}

void il_Script_printStack(lua_State *L, const char* Str) {
  int J, Top;
  printf("%-26s [", Str);
  Top = lua_gettop(L);
  for (J = 1; J <= Top; J++){
    switch (lua_type(L, J)) {
      case LUA_TNONE:
        printf("???, ");
        break;
      case LUA_TNIL:
        printf("nil, ");
        break;
      case LUA_TNUMBER:
        printf("%f, ", lua_tonumber(L, J));
        break;
      case LUA_TSTRING:
        printf("\"%s\", ", lua_tolstring(L, J, NULL));
      case LUA_TBOOLEAN:
        if (lua_toboolean(L, J))
          printf("true, ");
        else
          printf("false, ");
        break;
      case LUA_TTHREAD:
      case LUA_TLIGHTUSERDATA:
      case LUA_TTABLE:
      case LUA_TUSERDATA:
      case LUA_TFUNCTION:
        printf("%s: %p, ", lua_typename(L, lua_type(L, J)), lua_topointer(L, J));
        break;
    }
  }
  printf("]\n");
}

struct lua_node {
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
