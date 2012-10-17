#include "common/positionable.h"

#include "script/script.h"
#include "script/il.h"
#include "common/string.h"

extern int sg_Vector3_wrap(lua_State* L, sg_Vector3 v);

static int pos_index(lua_State* L) {
  il_Common_Positionable* self = il_Script_getPointer(L, 1, "positionable", NULL);
  il_Common_String k = il_Script_getString(L, 2);
  
  if (il_strcmp(k, il_l("position"))) {
    return sg_Vector3_wrap(L, self->position);
  }
  if (il_strcmp(k, il_l("size"))) {
    return sg_Vector3_wrap(L, self->size);
  }
  if (il_strcmp(k, il_l("velocity"))) {
    return sg_Vector3_wrap(L, self->velocity);
  }
  if (il_strcmp(k, il_l("parent"))) {
    return 0; // TODO: add world wrapper here
  }
  
  lua_getglobal(L, "positionable");
  lua_pushvalue(L, 2);
  lua_gettable(L, -2);
  
  return 1;
}

static int pos_newindex(lua_State* L) {
  il_Common_Positionable* self = il_Script_getPointer(L, 1, "positionable", NULL);
  il_Common_String k = il_Script_getString(L, 2);
  
  const char * type = il_Script_getType(L, 3);
  
  if (strcmp(type, "vector3") == 0) {
    sg_Vector3* v = il_Script_getPointer(L, 3, "vector3", NULL);
    if (il_strcmp(k, il_l("position"))) {
      self->position = *v;
      return 0;
    }
    if (il_strcmp(k, il_l("size"))) {
      self->size = *v;
      return 0;
    }
    if (il_strcmp(k, il_l("velocity"))) {
      self->velocity = *v;
      return 0;
    }
  }
  if (il_strcmp(k, il_l("parent"))) {
    self->parent = il_Script_getPointer(L, 3, "world", NULL);
    return 0;
  }
  return 0;
}

static int pos_create(lua_State* L) {
  void * world = il_Script_getPointer(L, 1, "world", NULL);
  il_Common_Positionable* p = il_Common_Positionable_new(world);
  
  il_Script_createMakeLight(L, p, "positionable");
  lua_pushcfunction(L, &pos_index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, &pos_newindex);
  lua_setfield(L, -2, "__newindex");
  return il_Script_createEndMt(L);
}

void il_Common_Positionable_luaGlobals(il_Script_Script* self, void * ctx) {
  il_Script_startTable(self);
  
  il_Script_addFunc(self, "create", &pos_create);
  il_Script_addTypeGetter(self, "positionable");
  il_Script_addIsA(self, "positionable");
  
  il_Script_startMetatable(self, "positionable", &pos_create);
  
  il_Script_addFunc(self, "__index", &pos_index);
  il_Script_addFunc(self, "__newindex", &pos_newindex);
  
  il_Script_endMetatable(self);
}
