#include "graphics/drawable3d.h"
#include "graphics/shape.h"
#include "graphics/terrain.h"

#include <string.h>

#include "script/script.h"
#include "script/il.h"
#include "interface.h"
#include "common/terrain.h"
#include "graphics/glutil.h"

////////////////////////////////////////////////////////////////////////////////
// Drawable

int il_Graphics_Drawable3d_wrap(lua_State* L, il_Graphics_Drawable3d* self) {
  return il_Script_createMakeLight(L, self, "drawable");
}

static int drawable_index(lua_State* L) {
  il_Graphics_Drawable3d* self = 
    il_Script_getChildT(L, 1, NULL, lua_upvalueindex(2));
  
  const char* k = luaL_checkstring(L, 2);
  
  if (strcmp(k, "type")) {
    lua_pushinteger(L, self->type);
    return 1;
  }
  if (strcmp(k, "positionable")) {
    il_Common_Positionable_wrap(L, self->positionable);
    return 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Terrain

struct il_Graphics_Terrain {
  il_Graphics_Drawable3d drawable;
  il_Common_Terrain * terrain;
  GLuint *buf;
  GLuint program;
  void *draw_ctx;
  void (*draw)(il_Graphics_Terrain*, void*, const il_Graphics_Camera*, 
    const struct timeval*);
};

int il_Graphics_Terrain_wrap(lua_State* L, il_Graphics_Terrain* self) {
  return il_Script_createMakeLight(L, self, "terrain");
}

static int terrain_create(lua_State* L) {
  il_Common_Terrain* ter = il_Script_getPointer(L, 1, "terraindata", NULL);
  il_Common_Positionable* pos = il_Script_getPointer(L, 2, "positionable", NULL);
  
  il_Graphics_Terrain* self = il_Graphics_Terrain_new(ter, pos);
  
  return il_Graphics_Terrain_wrap(L, self);
}

static int terrain_index(lua_State* L) {
  il_Graphics_Terrain* self = 
    il_Script_getChildT(L, 1, NULL, lua_upvalueindex(2));
  const char * k = luaL_checkstring(L, 2);
  
  if (strcmp(k, "data")) {
    return il_Common_Terrain_wrap(L, self->terrain);
  }

  return drawable_index(L);
}

////////////////////////////////////////////////////////////////////////////////
// Shape

struct il_Graphics_Shape {
  il_Graphics_Drawable3d drawable;
  int type;
  GLuint vbo;
  GLuint ibo;
  GLuint vao;
  GLuint program;
  GLuint texture;
  GLenum mode;
  GLsizei count;
};

int il_Graphics_Shape_wrap(lua_State* L, il_Graphics_Shape* self) {
  return il_Script_createMakeLight(L, self, "shape");
}

static const char * const shape_lst[] = {"box", "cylinder", "sphere", "plane"};

static int shape_create(lua_State* L) {
  int type = luaL_checkoption(L, 1, "box", shape_lst);
  il_Common_Positionable* pos;
  if (!lua_isnone(L, 2))
    pos = il_Script_getPointer(L, 1, "positionable", NULL);
  else
    pos = il_Common_Positionable_new(il_Script_getPointer(L, 1, "world", NULL));
  
  il_Graphics_Shape* self = il_Graphics_Shape_new(pos, type);
  
  return il_Graphics_Shape_wrap(L, self);
}

static int shape_index(lua_State* L) {
  il_Graphics_Shape* self = il_Script_getPointer(L, 1, "shape", NULL);
  const char * k = luaL_checkstring(L, 2);
  
  if (strcmp(k, "type")) {
    const char * const t = shape_lst[self->type];
    lua_pushlstring(L, t, strlen(t));
    return 1;
  }
  
  return drawable_index(L);
}

////////////////////////////////////////////////////////////////////////////////

void il_Graphics_Drawable3d_luaGlobals(il_Script_Script* self, void * ctx) {
  (void)ctx;
  
  //////////////
  // Drawable

  const luaL_Reg l[] = {
    {"getType", &il_Script_typeGetter},
    {"isA", &il_Script_isA},
    
    {NULL, NULL}
  };
  
  il_Script_startTable(self, l);
  
  il_Script_startMetatable(self, "drawable");
  il_Script_pushFunc(self->L, "__index", &drawable_index);
  
  il_Script_typeTable(self->L, "drawable");
  
  il_Script_endTable(self, l, "drawable");
  
  /////////////
  // Terrain
  
  const luaL_Reg terrain[] = {
    {"create", &terrain_create},
    
    {NULL, NULL}
  };
  
  il_Script_startTable(self, terrain);
  
  il_Script_startMetatable(self, "terrain");
  il_Script_pushFunc(self->L, "__index", &terrain_index);
  
  il_Script_typeTable(self->L, "terrain", "drawable");
  
  luaL_setfuncs(self->L, l, 2); // put the "parent" functions into this table
  
  // continue as usual
  
  il_Script_startMetatable(self, "terrain");
  
  il_Script_typeTable(self->L, "terrain", "drawable");
  
  il_Script_endTable(self, l, "terrain");
  
  ///////////
  // Shape
  
  const luaL_Reg shape[] = {
    {"create", &shape_create},
    
    {NULL, NULL}
  };
  
  il_Script_startTable(self, shape);
  
  il_Script_startMetatable(self, "shape");
  il_Script_pushFunc(self->L, "__index", &shape_index);
  
  il_Script_typeTable(self->L, "shape", "drawable");
  
  luaL_setfuncs(self->L, l, 2); // put the "parent" functions into this table
  
  // continue as usual
  
  il_Script_startMetatable(self, "shape");
  
  il_Script_typeTable(self->L, "shape", "drawable");
  
  il_Script_endTable(self, l, "shape");
  
}
