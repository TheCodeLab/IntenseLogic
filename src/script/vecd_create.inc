#define _c(a,b) a##b
#define c(a,b) _c(a,b)

#define _s(a) #a
#define s(a) _s(a)

#define vecd c(sg_Vector, d)

static int c(vec,c(d,_create))(lua_State* L) {
  int nargs = lua_gettop(L);
  float x, y;
  x = 0; y = 0;
  #if d > 2
  float z = 0;
  #endif
  #if d > 3
  float w = 0;
  #endif
  
  //ilS_printStack(L, "vec"s(d)"_create");
  
  lua_getglobal(L, "vector" s(d)); // special case for self parameter passed with : syntax and __call
  if (nargs >= 1 && lua_compare(L, 1, -1, LUA_OPEQ)) {
    nargs--;
    lua_remove(L, 1);
  }
  lua_pop(L, 1);
  
  switch (nargs) {
    case 0:
      break;
    case 1: // number
      x = lua_tonumber(L, 1);
      y = lua_tonumber(L, 1);
      #if d > 2
      z = lua_tonumber(L, 1);
      #endif
      #if d > 3
      w = lua_tonumber(L, 1);
      #endif
      break;
    case d: // number
      if (    lua_type(L, 1) == LUA_TNUMBER 
          &&  lua_type(L, 2) == LUA_TNUMBER
          #if d > 2
          &&  lua_type(L, 3) == LUA_TNUMBER
          #endif
          #if d > 3
          &&  lua_type(L, 4) == LUA_TNUMBER
          #endif
          ) {
        x = lua_tonumber(L, 1);
        y = lua_tonumber(L, 2);
        #if d > 2
        z = lua_tonumber(L, 3);
        #endif
        #if d > 3
        w = lua_tonumber(L, 4);
        #endif
      } else goto error;
      break;
    default:
      goto error;
  }
  
  vecd v = (vecd){
    x,
    y,
    #if d > 2
    z,
    #endif
    #if d > 3
    w
    #endif
  };
  return c(vec, c(d, _wrap))(L, v);
  
  error:
  luaL_argerror(L, 1, "Expected " s(d) " numbers, or nil");
  return -1;
}

#undef _c
#undef c
#undef s
#undef vecd
