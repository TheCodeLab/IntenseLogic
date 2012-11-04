#include <stdint.h>
#include "script/script.h"

typedef void (*il_Script_LuaRegisterFunc)(il_Script_Script*, void * ctx);

#define il_Script_startTable(self, l) luaL_newlibtable(self->L, l)
int il_Script_startMetatable(il_Script_Script* self, const char * name);
int il_Script_pushFunc(lua_State* L, const char * name, lua_CFunction func);
int il_Script_typeTable(lua_State* L, const char * str, ...);
int il_Script_endTable(il_Script_Script* self, const luaL_Reg* l, const char * name);

int il_Script_typeGetter(lua_State* L);
int il_Script_isA(lua_State* L);

int il_Script_createMakeLight(lua_State* L, void * ptr, const char * type);
int il_Script_createMakeHeavy(lua_State* L, size_t size, const void * ptr, const char * type);

const char * il_Script_getType(lua_State* L, int idx);
void* il_Script_getPointer(lua_State* L, int idx, const char * type, size_t *size);
void* il_Script_getChild(lua_State* L, int idx, size_t *size, const char * type, ...);
void* il_Script_getChildT(lua_State* L, int idx, size_t *size, int tidx);
il_Common_String il_Script_getString(lua_State* L, int idx);
double il_Script_getNumber(lua_State* L, int idx);
il_Common_String il_Script_toString(lua_State* L, int idx);
void il_Script_printStack(lua_State *L, const char* Str);

void il_Script_registerLuaRegister(il_Script_LuaRegisterFunc func, void * ctx);
void il_Script_openLibs(il_Script_Script*);
