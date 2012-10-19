#define _c(a,b) a##b
#define c(a,b) _c(a,b)

#define _s(a) #a
#define s(a) _s(a)

#define vecd c(sg_Vector, d)

il_Script_startTable(self);
il_Script_addFunc(self, "create", &c(vec, c(d, _create)));
il_Script_addTypeGetter(self, "vector" s(d));
il_Script_addIsA(self, "vector" s(d));

il_Script_addFunc(self, "dot", &c(vec, c(d, _dot)));
#if d > 2
il_Script_addFunc(self, "cross", &c(vec, c(d, _cross)));
#endif

il_Script_startMetatable(self, "vector" s(d), &c(vec, c(d, _create)));
il_Script_addFunc(self, "__add", &c(vec, c(d, _add)));
il_Script_addFunc(self, "__sub", &c(vec, c(d, _sub)));
il_Script_addFunc(self, "__mul", &c(vec, c(d, _mul)));
il_Script_addFunc(self, "__div", &c(vec, c(d, _div)));

il_Script_endMetatable(self);