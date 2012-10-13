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
int il_Script_createEndMt(lua_State* L);
void* il_Script_getPointer(lua_State* L, int idx, const char * type, size_t *size);
il_Common_String il_Script_getString(lua_State* L, int idx);
void il_Script_printStack(lua_State *L, const char* Str);

void il_Script_registerLuaRegister(il_Script_LuaRegisterFunc func, void * ctx);
void il_Script_openLibs(il_Script_Script*);
