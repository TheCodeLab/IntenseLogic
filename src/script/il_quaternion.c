#include "common/quaternion.h"

#include "script/script.h"
#include "script/il.h"

int sg_Quaternion_wrap(lua_State* L, sg_Quaternion q);

static int quat_index(lua_State* L) {
  sg_Quaternion* q = il_Script_getPointer(L, 1, "quaternion", NULL);
  il_String k = il_Script_getString(L, 2);
  
  if (il_strcmp(k, il_l("x"))) {
    lua_pushnumber(L, q->x);
    return 1;
  }
  if (il_strcmp(k, il_l("y"))) {
    lua_pushnumber(L, q->y);
    return 1;
  }
  if (il_strcmp(k, il_l("z"))) {
    lua_pushnumber(L, q->z);
    return 1;
  }
  if (il_strcmp(k, il_l("w"))) {
    lua_pushnumber(L, q->w);
    return 1;
  }
  
  lua_getglobal(L, "quaternion");
  lua_pushvalue(L, 2);
  lua_gettable(L, -2);
  
  return 1;
}

static int quat_newindex(lua_State* L) {
  sg_Quaternion* q = il_Script_getPointer(L, 1, "quaternion", NULL);
  il_String k = il_Script_getString(L, 2);
  double v = il_Script_getNumber(L, 3);
  
  if (il_strcmp(k, il_l("x"))) {
    q->x = v;
    return 0;
  }
  if (il_strcmp(k, il_l("y"))) {
    q->y = v;
    return 0;
  }
  if (il_strcmp(k, il_l("z"))) {
    q->z = v;
    return 0;
  }
  if (il_strcmp(k, il_l("w"))) {
    q->w = v;
    return 0;
  }
  
  return 0;
}

static int quat_mul(lua_State* L) {
  sg_Quaternion* a = il_Script_getPointer(L, 1, "quaternion", NULL);
  sg_Quaternion* b = il_Script_getPointer(L, 2, "quaternion", NULL);
  sg_Quaternion res = sg_Quaternion_mul(*a, *b);
  return sg_Quaternion_wrap(L, res);
}

static int quat_fromAxisAngle(lua_State* L) {
  int nargs = lua_gettop(L);
  sg_Vector3 v;
  float a;
  if (nargs == 4) {
    v.x = il_Script_getNumber(L, 1);
    v.y = il_Script_getNumber(L, 2);
    v.z = il_Script_getNumber(L, 3);
    a = il_Script_getNumber(L, 4);
  } else if (nargs == 2) {
    v = *(sg_Vector3*)il_Script_getPointer(L, 1, "vector3", NULL);
    a = il_Script_getNumber(L, 2);
  } else luaL_argerror(L, 1, "Expected 4 numbers or a vector and a number");
  sg_Quaternion q = sg_Quaternion_fromAxisAngle(v, a);
  return sg_Quaternion_wrap(L, q);
}

static int quat_fromYPR(lua_State* L) {
  float y, p, r;
  int nargs = lua_gettop(L);
  if (nargs != 3) luaL_argerror(L, 1, "Expected 3 numbers");
  y = il_Script_getNumber(L, 1);
  p = il_Script_getNumber(L, 2);
  r = il_Script_getNumber(L, 3);
  sg_Quaternion q = sg_Quaternion_fromYPR(y, p, r);
  return sg_Quaternion_wrap(L, q);
}

int sg_Quaternion_wrap(lua_State* L, sg_Quaternion q) {
  il_Script_createMakeHeavy(L, sizeof(sg_Quaternion), &q, "quaternion");
  
  lua_pushcfunction(L, &quat_index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, &quat_newindex);
  lua_setfield(L, -2, "__newindex");
  lua_pushcfunction(L, &quat_mul);
  lua_setfield(L, -2, "__mul");
  
  return il_Script_createEndMt(L);
}

static int quat_create(lua_State* L) {
  sg_Quaternion q = (sg_Quaternion){0, 0, 0, 1};
  int nargs = lua_gettop(L);
  if (nargs == 4) {
    q.x = il_Script_getNumber(L, 1);
    q.y = il_Script_getNumber(L, 2);
    q.z = il_Script_getNumber(L, 3);
    q.w = il_Script_getNumber(L, 4);
  } else if (nargs != 0) luaL_argerror(L, 1, "Expected 4 numbers or nothing");
  return sg_Quaternion_wrap(L, q);
}

void sg_Quaternion_luaGlobals(il_Script_Script* self, void * ctx) {
  il_Script_startTable(self);
  
  il_Script_addFunc(self, "create", &quat_create);
  il_Script_addTypeGetter(self, "quaternion");
  il_Script_addIsA(self, "quaternion");
  
  il_Script_addFunc(self, "fromAxisAngle", &quat_fromAxisAngle);
  il_Script_addFunc(self, "fromYPR", &quat_fromYPR);
  
  il_Script_startMetatable(self, "quaternion", &quat_create);
  
  il_Script_endMetatable(self);
}
