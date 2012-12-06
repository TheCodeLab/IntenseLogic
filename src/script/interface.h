#ifndef IL_SCRIPT_INTERFACE_H
#define IL_SCRIPT_INTERFACE_H

#include "common/vector.h"
#include "common/quaternion.h"
#include "common/positionable.h"
#include "common/world.h"
#include "common/event.h"
#include "common/terrain.h"
#include "graphics/drawable3d.h"
#include "graphics/terrain.h"
#include "graphics/world.h"

void il_Vector_luaGlobals(ilS_script* self, void* ctx);
void ilS_luaGlobals(ilS_script* self, void * ctx);
void il_positionable_luaGlobals(ilS_script* self, void * ctx);
void ilG_world_luaGlobals(ilS_script* self, void * ctx);
void il_Quaternion_luaGlobals(ilS_script* self, void * ctx);
void ilE_luaGlobals(ilS_script* self, void * ctx);
void ilI_luaGlobals(ilS_script* self, void * ctx);
void il_terrain_luaGlobals(ilS_script* self, void * ctx);
void ilA_luaGlobals(ilS_script* self, void * ctx);
void ilG_drawable3d_luaGlobals(ilS_script* self, void * ctx);
void ilG_camera_luaGlobals(ilS_script* self, void * ctx);
void ilG_drawable3d_luaGlobals(ilS_script* self, void * ctx);

int il_Vector2_wrap(lua_State* L, il_Vector2 v);
int il_Vector3_wrap(lua_State* L, il_Vector3 v);
int il_Vector4_wrap(lua_State* L, il_Vector4 v);
int il_Quaternion_wrap(lua_State* L, il_Quaternion q);
int il_positionable_wrap(lua_State* L, il_positionable* p);
int ilS_script_wrap(lua_State* L, ilS_script* s);
int ilG_world_wrap(lua_State* L, ilG_world * w);
int ilE_wrap(lua_State* L, const ilE_event* e);
int il_terrain_wrap(lua_State* L, il_terrain* ter);
int ilA_wrap(lua_State* L, ilA_asset* asset);
int ilG_drawable3d_wrap(lua_State* L, ilG_drawable3d*);
int ilG_camera_wrap(lua_State* L, ilG_camera* self);
int ilG_terrain_wrap(lua_State* L, ilG_terrain* self);
int il_world_wrap(lua_State* L, il_world * self);


#endif
