#include "script/script.h"

typedef void (*il_Script_LuaRegisterFunc)(il_Script_Script*);

int il_Script_startTable(il_Script_Script*);
int il_Script_addFunc(il_Script_Script*, const char * name, lua_CFunction func);
int il_Script_addClosure(il_Script_Script*, const char * name, lua_CFunction func, int n);
int il_Script_addTypeGetter(il_Script_Script* self, const char * type);
#define il_Script_addIsA(self, ...) il_Script_addIsAfunc(self, ##__VA_ARGS__, NULL);
int il_Script_addIsAfunc(il_Script_Script* self, const char * type, ...);
int il_Script_endTable(il_Script_Script* self, const char * name, lua_CFunction call);

int il_Script_createHelper(lua_State* L, void * ptr, const char * type);
void* il_Script_getPointer(lua_State* L, int idx, const char * type);
il_Common_String il_Script_getString(lua_State* L, int idx);
