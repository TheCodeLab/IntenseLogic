#include "graphics/camera.h"

#include "script/script.h"
#include "script/il.h"
#include "interface.h"

static int camera_index(lua_State* L) {
  il_Graphics_Camera* self = il_Script_getPointer(L, 1, "camera", NULL);
  const char * key = luaL_checkstring(L, 2);
  
  if (strcmp(key, "positionable") == 0) {
    return il_Common_Positionable_wrap(L, self->positionable);
  }
  if (strcmp(key, "movespeed") == 0) {
    return sg_Vector3_wrap(L, self->movespeed);
  }
  if (strcmp(key, "sensitivity") == 0) {
    lua_pushnumber(L, self->sensitivity);
    return 1;
  }
  // TODO: bind projection_matrix
  return 0;
}

static int newindex(lua_State* L) {
  il_Graphics_Camera* self = il_Script_getPointer(L, 1, "camera", NULL);
  const char *key = luaL_checkstring(L, 2);
  
  if (strcmp(key, "positionable") == 0) {
    self->positionable = il_Script_getPointer(L, 3, "positionable", NULL);
    return 0;
  }
  if (strcmp(key, "movespeed") == 0) {
    self->movespeed = *(sg_Vector3*)il_Script_getPointer(L, 3, "vector3", NULL);
    return 0;
  }
  if (strcmp(key, "sensitivity") == 0) {
    self->sensitivity = luaL_checknumber(L, 3);
    return 0;
  }
  // TODO: bind projection matrix
  return 0;
}

int il_Graphics_Camera_wrap(lua_State* L, il_Graphics_Camera* self) {
  return il_Script_createMakeLight(L, self, "camera");
}

static int create(lua_State* L) {
  il_Common_Positionable* pos;
  if (luaL_testudata(L, 1, "positionable")) {
    pos = il_Script_getPointer(L, 1, "positionable", NULL);
  } else if (luaL_testudata(L, 1, "world")) {
    pos = il_Common_Positionable_new(il_Script_getPointer(L, 1, "world", NULL));
  } else {
    // TODO: figure out the current world somehow
    luaL_argerror(L, 1, "Expected positionable or world");
  }
  il_Graphics_Camera* cam = il_Graphics_Camera_new(pos);
  return il_Graphics_Camera_wrap(L, cam);
}

void il_Graphics_Camera_luaGlobals(il_Script_Script* self, void * ctx) {
  (void)ctx;
  
  luaL_Reg l[] = {
    {"create", &create},
    {"getType", &il_Script_typeGetter},
    {"isA", &il_Script_isA},
    
    {NULL, NULL}
  };
  
  il_Script_startTable(self, l);
  
  il_Script_startMetatable(self, "camera");
  il_Script_pushFunc(self->L, "__index", &camera_index);
  il_Script_pushFunc(self->L, "__newidnex", &newindex);
  
  il_Script_typeTable(self->L, "camera");
  
  il_Script_endTable(self, l, "camera");
}
