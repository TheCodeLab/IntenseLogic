#define _c(a,b) a##b
#define c(a,b) _c(a,b)

#define _s(a) #a
#define s(a) _s(a)

#define vecd c(sg_Vector, d)

#define mt_fun(name,d) \
  lua_pushcfunction(self->L, &c(vec, c(d, c(_, name)))); \
  lua_setfield(self->L, -2, "__" s(name));

const luaL_Reg c(l,d)[] = {
  {"create",    &c(vec, c(d, _create))},
  {"getType",   &il_Script_typeGetter},
  {"isA",       &il_Script_isA},
  
  {"dot",       &c(vec, c(d, _dot))},
  #if d > 2
  {"cross",     &c(vec, c(d, _cross))},
  #endif
  
  {NULL,        NULL},
};

il_Script_startTable(self, c(l,d));

il_Script_startMetatable(self, "vector" s(d));
mt_fun(add, d); 
mt_fun(sub, d); 
mt_fun(mul, d); 
mt_fun(div, d); 
mt_fun(tostring, d); 
mt_fun(index, d); 
mt_fun(newindex, d);

il_Script_typeTable(self->L, "vector" s(d));

il_Script_endTable(self, c(l,d), "vector" s(d));
