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
