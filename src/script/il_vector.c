// Copyright (c) 2012 Code Lab
//
// This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
//
//     2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
//
//     3. This notice may not be removed or altered from any source distribution.
//

#include "common/vector.h"
#include "script/il.h"
#include "script/script.h"

static int vec2_wrap(lua_State* L, sg_Vector2 v);

#define simple_op(name, op) \
static int vec2_##name(lua_State* L) { \
  sg_Vector2* a = (sg_Vector2*)il_Script_getPointer(L, 1, "vector2", NULL); \
  sg_Vector2 res; \
  if (lua_isnumber(L, 2)) { \
    float b = (float)lua_tonumber(L, 2); \
    res = sg_Vector2_##name##_f(*a, b); \
  } else { \
    sg_Vector2* b = (sg_Vector2*)il_Script_getPointer(L, 2, "vector2", NULL); \
    res = sg_Vector2_##name(*a, *b); \
  } \
  return vec2_wrap(L, res); \
  \
}

simple_op(add, +)
simple_op(sub, -)
simple_op(mul, *)
simple_op(div, /)

#undef simple_op

static int vec2_tostring(lua_State* L) {
  sg_Vector2* v = (sg_Vector2*)il_Script_getPointer(L, 1, "vector2", NULL);
  
  lua_pushfstring(L, "%f, %f", (lua_Number)v->x, (lua_Number)v->y);
  
  return 1;
}

static int vec2_index(lua_State* L) {
  sg_Vector2* v = (sg_Vector2*)il_Script_getPointer(L, 1, "vector2", NULL);
  const char * k = lua_tolstring(L, 2, NULL);
  
  if (k != NULL) {
    if (strcmp(k, "x") == 0) {
      lua_pushnumber(L, v->x);
      return 1;
    }
    if (strcmp(k, "y") == 0) {
      lua_pushnumber(L, v->y);
      return 1;
    }
    if (strcmp(k, "len") == 0 || strcmp(k, "length") == 0) {
      lua_pushnumber(L, sg_Vector2_len(*v));
      return 1;
    }
  }
  
  lua_getglobal(L, "vector2");
  lua_pushvalue(L, 2);
  lua_gettable(L, -2);
  
  return 1;
}

static int vec2_newindex(lua_State* L) {
  sg_Vector2* t = (sg_Vector2*)il_Script_getPointer(L, 1, "vector2", NULL);
  if (!lua_isstring(L, 2)) return luaL_error(L, "Expected string key");
  const char * k = lua_tolstring(L, 2, NULL);
  if (!lua_isnumber(L, 3)) return luaL_error(L, "Expected number value");
  float v = lua_tonumber(L, 3);
  
  if (strcmp(k, "x") == 0) {
    t->x = v;
    return 0;
  }
  if (strcmp(k, "y") == 0) {
    t->y = v;
    return 0;
  }
  return luaL_error(L, "Invalid key");
}

static int vec2_wrap(lua_State* L, sg_Vector2 v) {
  int idx = il_Script_createMakeHeavy(L, sizeof(sg_Vector2), &v, "vector2");
  
  il_Script_printStack(L, "vec2_wrap");
  
  #define mt_fun(name) \
    lua_pushcfunction(L, &vec2_##name); \
    lua_setfield(L, idx, "__" #name);
  
  mt_fun(add);
  mt_fun(sub);
  mt_fun(mul);
  mt_fun(div);
  mt_fun(tostring);
  mt_fun(index);
  mt_fun(newindex);
  
  #undef mt_fun
  
  return il_Script_createEndMt(L);
}

static int vec2_create(lua_State* L) {
  int nargs = lua_gettop(L);
  //il_Script_printStack(L, "vector2.create");
  float x, y;
  x = 0; y = 0;
  
  lua_getglobal(L, "vector2"); // special case for self parameter passed with : syntax and __call
  if (nargs >= 1 && lua_equal(L, 1, -1)) {
    nargs--;
    lua_remove(L, 1);
  }
  lua_pop(L, 1);
  
  //printf("nargs: %i\n", nargs);
  
  switch (nargs) {
    case 0:
      break;
    case 1: // either a table or a number
      switch(lua_type(L, 1)) {
        case LUA_TTABLE: {
          // x = t[1]
          lua_pushinteger(L, 1);
          lua_gettable(L, 1);
          if (lua_isnumber(L, -1))
            x = lua_tonumber(L, -1);
          lua_pop(L, 1);
          
          // y = t[2]
          lua_pushinteger(L, 2);
          lua_gettable(L, 1);
          if (lua_isnumber(L, -1))
            y = lua_tonumber(L, -1);
          lua_pop(L, 1);
          
          // x = t.x
          lua_getfield(L, 1, "x");
          if (lua_isnumber(L, -1))
            x = lua_tonumber(L, -1);
          lua_pop(L, 1);
          
          // y = t.y
          lua_getfield(L, 1, "y");
          if (lua_isnumber(L, -1))
            y = lua_tonumber(L, -1);
          lua_pop(L, 1);
          
          break;
        }
        case LUA_TNUMBER:
          x = lua_tonumber(L, 1);
          y = lua_tonumber(L, 2);
          break;
        default:
          goto error;
      }
      break;
    case 2: // number
      if (lua_type(L, 1) == LUA_TNUMBER && lua_type(L, 1) == LUA_TNUMBER) {
        x = lua_tonumber(L, 1);
        y = lua_tonumber(L, 2);
      } else goto error;
      break;
    default:
      goto error;
  }
  
  sg_Vector2 v = (sg_Vector2){x,y};
  return vec2_wrap(L, v);
  
  error:
  luaL_argerror(L, 1, "Expected table, 2 numbers, or nothing");
  return -1;
}

void sg_Vector_luaGlobals(il_Script_Script* self, void* ctx) {
  // vector2
  il_Script_startTable(self);
  
  il_Script_addFunc(self, "create", &vec2_create);
  il_Script_addTypeGetter(self, "vector2");
  il_Script_addIsA(self, "vector2");
  
  
  
  il_Script_startMetatable(self, "vector2", &vec2_create);
  
  il_Script_addFunc(self, "__add", &vec2_add);
  il_Script_addFunc(self, "__sub", &vec2_sub);
  il_Script_addFunc(self, "__mul", &vec2_mul);
  il_Script_addFunc(self, "__div", &vec2_div);
  
  il_Script_endMetatable(self);
}
