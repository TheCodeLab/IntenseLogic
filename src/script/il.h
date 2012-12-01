#include <stdint.h>
#include "script/script.h"

typedef void (*ilS_luaRegisterFunc)(ilS_script*, void * ctx);

#define ilS_startTable(self, l) luaL_newlibtable(self->L, l)
int ilS_startMetatable(ilS_script* self, const char * name);
int ilS_pushFunc(lua_State* L, const char * name, lua_CFunction func);
int ilS_typeTable(lua_State* L, const char * str, ...);
int ilS_endTable(ilS_script* self, const luaL_Reg* l, const char * name);

int ilS_typeGetter(lua_State* L);
int ilS_isA(lua_State* L);

int ilS_createMakeLight(lua_State* L, void * ptr, const char * type);
int ilS_createMakeHeavy(lua_State* L, size_t size, const void * ptr, const char * type);

const char * ilS_getType(lua_State* L, int idx);
void* ilS_getPointer(lua_State* L, int idx, const char * type, size_t *size);
void* ilS_getChild(lua_State* L, int idx, size_t *size, const char * type, ...);
void* ilS_getChildT(lua_State* L, int idx, size_t *size, int tidx);
il_string ilS_getString(lua_State* L, int idx);
double ilS_getNumber(lua_State* L, int idx);
il_string ilS_toString(lua_State* L, int idx);
void ilS_printStack(lua_State *L, const char* Str);

void ilS_registerLuaRegister(ilS_luaRegisterFunc func, void * ctx);
void ilS_openLibs(ilS_script*);
