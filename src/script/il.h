#include <stdint.h>
#include "script/script.h"

typedef void (*il_Script_LuaRegisterFunc)(il_Script_Script*, void * ctx);

typedef struct il_Script_TypedPointer {
  int is_pointer;
  const char * type;
  void * ptr;
} il_Script_TypedPointer;

typedef struct il_Script_TypedBox {
  int is_pointer;
  const char * type;
  size_t size;
  uint8_t data[];
} il_Script_TypedBox;

int il_Script_startTable(il_Script_Script*);
int il_Script_addFunc(il_Script_Script*, const char * name, lua_CFunction func);
int il_Script_addClosure(il_Script_Script*, const char * name, lua_CFunction func, int n);
int il_Script_addTypeGetter(il_Script_Script* self, const char * type);
#define il_Script_addIsA(self, ...) il_Script_addIsAfunc(self, ##__VA_ARGS__, NULL);
int il_Script_addIsAfunc(il_Script_Script* self, const char * type, ...);
int il_Script_startMetatable(il_Script_Script* self, const char * name, lua_CFunction call);
int il_Script_endMetatable(il_Script_Script* self);

int il_Script_createMakeLight(lua_State* L, void * ptr, const char * type);
int il_Script_createMakeHeavy(lua_State* L, size_t size, const void * ptr, const char * type);
int il_Script_createAddFunc(lua_State* L, const char * name, lua_CFunction func);
int il_Script_createEndMt(lua_State* L);
const char * il_Script_getType(lua_State* L, int idx);
void* il_Script_getPointer(lua_State* L, int idx, const char * type, size_t *size);
il_Common_String il_Script_getString(lua_State* L, int idx);
double il_Script_getNumber(lua_State* L, int idx);
void il_Script_printStack(lua_State *L, const char* Str);

void il_Script_registerLuaRegister(il_Script_LuaRegisterFunc func, void * ctx);
void il_Script_openLibs(il_Script_Script*);
