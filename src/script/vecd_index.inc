
#define _c(a,b) a##b
#define c(a,b) _c(a,b)

#define _s(a) #a
#define s(a) _s(a)

#define vecd c(sg_Vector, d)

static int c(vec,c(d,_index))(lua_State* L) {
  vecd* v = (vecd*)il_Script_getPointer(L, 1, "vector" s(d), NULL);
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
    #if d > 2
    if (strcmp(k, "z") == 0) {
      lua_pushnumber(L, v->z);
      return 1;
    }
    #endif
    #if d > 3
    if (strcmp(k, "w") == 0) {
      lua_pushnumber(L, v->z);
      return 1;
    }
    #endif
    if (strcmp(k, "len") == 0 || strcmp(k, "length") == 0) {
      lua_pushnumber(L, c(vecd,_len)(*v));
      return 1;
    }
    if (strcmp(k, "normal") == 0) {
      vecd res = c(vecd, _normalise)(*v);
      return c(vec, c(d, _wrap))(L, res);
    }
  }
  
  lua_getglobal(L, "vector" s(d));
  lua_pushvalue(L, 2);
  lua_gettable(L, -2);
  
  return 1;
}

static int c(vec, c(d,_newindex))(lua_State* L) {
  vecd* t = (vecd*)il_Script_getPointer(L, 1, "vector" s(d), NULL);
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
  #if d > 2
  if (strcmp(k, "z") == 0) {
    t->z = v;
    return 0;
  }
  #endif
  #if d > 3
  if (strcmp(k, "w") == 0) {
    t->w = v;
    return 0;
  }
  #endif
  return luaL_error(L, "Invalid key");
}

#undef _c
#undef c
#undef s
#undef vecd
