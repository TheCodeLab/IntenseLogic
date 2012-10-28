#include "common/terrain.h"

#include <math.h>

#include "script/script.h"
#include "script/il.h"
#include "common/string.h"
#include "common/vector.h"
#include "script/interface.h"

int il_Common_Terrain_wrap(lua_State* L, il_Common_Terrain* ter) {
  return il_Script_createMakeLight(L, ter, "terrain");
}

static int create(lua_State* L) {
  return il_Common_Terrain_wrap(L, il_Common_Terrain_new());
}

static int ter_index(lua_State* L) {
  il_Common_Terrain* ter = (il_Common_Terrain*)il_Script_getPointer(L, 1, "terrain", NULL);
  il_Common_String k = il_Script_getString(L, 2);
  
  if (il_strcmp(k, il_l("width"))) {
    int w;
    il_Common_Terrain_getSize(ter, &w, NULL);
    lua_pushinteger(L, w);
    return 1;
  }
  if (il_strcmp(k, il_l("height"))) {
    int h;
    il_Common_Terrain_getSize(ter, &h, NULL);
    lua_pushinteger(L, h);
    return 1;
  }
  
  return 0;
}

static int getpoint(lua_State* L) {
  il_Common_Terrain* ter = (il_Common_Terrain*)il_Script_getPointer(L, 1, "terrain", NULL);
  unsigned x = luaL_checkunsigned(L, 2);
  unsigned y = luaL_checkunsigned(L, 3);
  double z = luaL_optnumber(L, 4, NAN);
  double ret = il_Common_Terrain_getPoint(ter, x, y, z);
  lua_pushnumber(L, ret);
  return 1;
}

static int getnormal(lua_State* L) {
  il_Common_Terrain* ter = (il_Common_Terrain*)il_Script_getPointer(L, 1, "terrain", NULL);
  unsigned x = luaL_checkunsigned(L, 2);
  unsigned y = luaL_checkunsigned(L, 3);
  double z = luaL_optnumber(L, 4, NAN);
  sg_Vector3 ret = il_Common_Terrain_getNormal(ter, x, y, z);
  sg_Vector3_wrap(L, ret);
  return 1;
}

static int heightmap(lua_State* L) {
  il_Common_Terrain* ter = (il_Common_Terrain*)il_Script_getPointer(L, 1, "terrain", NULL);
  int w = luaL_checkinteger(L, 2);
  int h = luaL_checkinteger(L, 3);
  if (!lua_istable(L, 4)) luaL_argerror(L, 4, "Expected table");
  if (lua_rawlen(L, 4) < (size_t)w*(size_t)h) 
    luaL_argerror(L, 4, "Expected table with at least W x H elements");
  
  float* map = calloc(sizeof(float), w*h);
  int x, y;
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      lua_rawgeti(L, 4, (y+1) * w + x+1);
      if (!lua_isnumber(L, -1)) {
        free(map);
        return luaL_error(L, "Invalid heightmap data");
      }
      double n = lua_tonumber(L, -1);
      map[y*w + x] = (float)n;
    }
  }
  il_Common_Terrain_heightmapFromMemory(ter, w, h, map);
  
  return 0;
}

void il_Common_Terrain_luaGlobals(il_Script_Script* self, void * ctx) {
  (void)ctx;

  luaL_Reg l[] = {
    {"create",        &create         },
    {"getType",       &il_Script_typeGetter},
    {"isA",           &il_Script_isA  },
    
    {"getPoint",      &getpoint       },
    {"getNormal",     &getnormal      },
    {"fromHeightmap", &heightmap      },
    
    {NULL,            NULL            }
  };
  
  il_Script_startTable(self, l);

  il_Script_startMetatable(self, "terrain");
  il_Script_pushFunc(self->L, "__index", &ter_index);
  
  il_Script_typeTable(self->L, "terrain");
  
  il_Script_endTable(self, l, "terrain");

}
