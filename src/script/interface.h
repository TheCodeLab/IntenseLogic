#ifndef IL_SCRIPT_INTERFACE_H
#define IL_SCRIPT_INTERFACE_H

#include "common/vector.h"
#include "common/quaternion.h"
#include "common/positionable.h"
#include "common/world.h"
#include "common/event.h"

void sg_Vector_luaGlobals(il_Script_Script* self, void* ctx);
void il_Script_luaGlobals(il_Script_Script* self, void * ctx);
void il_Common_Positionable_luaGlobals(il_Script_Script* self, void * ctx);
void il_Common_World_luaGlobals(il_Script_Script* self, void * ctx);
void sg_Quaternion_luaGlobals(il_Script_Script* self, void * ctx);
void il_Event_luaGlobals(il_Script_Script* self, void * ctx);

int sg_Vector2_wrap(lua_State* L, sg_Vector2 v);
int sg_Vector3_wrap(lua_State* L, sg_Vector3 v);
int sg_Vector4_wrap(lua_State* L, sg_Vector4 v);
int sg_Quaternion_wrap(lua_State* L, sg_Quaternion q);
int il_Common_Positionable_wrap(lua_State* L, il_Common_Positionable* p);
int il_Script_Script_wrap(lua_State* L, il_Script_Script* s);
int il_Common_World_wrap(lua_State* L, il_Common_World * w);
int il_Event_wrap(lua_State* L, const il_Event_Event* e);

#endif
