#include "common/terrain.h"

#include <math.h>

#include "script/script.h"
#include "script/il.h"
#include "common/string.h"
#include "common/vector.h"
#include "script/interface.h"

int il_terrain_wrap(lua_State* L, il_terrain* ter)
{
    return ilS_createMakeLight(L, ter, "terraindata");
}

static int create(lua_State* L)
{
    return il_terrain_wrap(L, il_terrain_new());
}

static int ter_index(lua_State* L)
{
    il_terrain* ter = ilS_getPointer(L, 1, "terraindata", NULL);
    il_string k = ilS_getString(L, 2);

    if (il_strcmp(k, il_l("width"))) {
        int w;
        il_terrain_getSize(ter, &w, NULL);
        lua_pushinteger(L, w);
        return 1;
    }
    if (il_strcmp(k, il_l("height"))) {
        int h;
        il_terrain_getSize(ter, &h, NULL);
        lua_pushinteger(L, h);
        return 1;
    }

    return 0;
}

static int getpoint(lua_State* L)
{
    il_terrain* ter = ilS_getPointer(L, 1, "terraindata", NULL);
    unsigned x = luaL_checkunsigned(L, 2);
    unsigned y = luaL_checkunsigned(L, 3);
    double z = luaL_optnumber(L, 4, NAN);
    double ret = il_terrain_getPoint(ter, x, y, z);
    lua_pushnumber(L, ret);
    return 1;
}

static int getnormal(lua_State* L)
{
    il_terrain* ter = ilS_getPointer(L, 1, "terraindata", NULL);
    unsigned x = luaL_checkunsigned(L, 2);
    unsigned y = luaL_checkunsigned(L, 3);
    double z = luaL_optnumber(L, 4, NAN);
    il_Vector3 ret = il_terrain_getNormal(ter, x, y, z);
    il_Vector3_wrap(L, ret);
    return 1;
}

static int heightmap(lua_State* L)
{
    il_terrain* ter = ilS_getPointer(L, 1, "terraindata", NULL);
    int w = luaL_checkinteger(L, 2);
    int h = luaL_checkinteger(L, 3);
    if (!lua_istable(L, 4)) luaL_argerror(L, 4, "Expected table");
    if (lua_rawlen(L, 4) < (size_t)w*(size_t)h)
        luaL_argerror(L, 4, "Expected table with at least W x H elements");

    float* map = calloc(w*h, sizeof(float));
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
    il_terrain_heightmapFromMemory(ter, w, h, map);

    return 0;
}

long long hash_seed(const char *s)
{
    long long ret = 0;
    size_t i;

    for (i = 0; i < strlen(s); i+=sizeof(long long)) {
        ret ^= *(long long*)(s+i);
    }
    long long b = 0;
    size_t l = strlen(s);
    memcpy(&b, s+l-(l%8), l%8);
    ret ^= b;
    return ret;
}

static int from_seed(lua_State* L)
{
    il_terrain* ter = ilS_getPointer(L, 1, "terraindata", NULL);

    if (!lua_isnumber(L,2) && !lua_isstring(L,2)) return luaL_argerror(L, 2, "Expected number or string");
    long long seed = lua_isnumber(L,2)? luaL_optlong(L, 2, 0) : hash_seed(luaL_checkstring(L, 2));

    float resolution = luaL_optnumber(L, 2, 1.0);
    float viewdistance = luaL_optnumber(L, 2, 100.0);

    il_terrain_heightmapFromSeed(ter, seed, resolution, viewdistance);

    return 0;
}

void il_terrain_luaGlobals(ilS_script* self, void * ctx)
{
    (void)ctx;

    luaL_Reg l[] = {
        {"create",        &create         },
        {"getType",       &ilS_typeGetter},
        {"isA",           &ilS_isA  },

        {"getPoint",      &getpoint       },
        {"getNormal",     &getnormal      },
        {"fromHeightmap", &heightmap      },
        {"fromSeed",      &from_seed      },

        {NULL,            NULL            }
    };

    ilS_startTable(self, l);

    ilS_startMetatable(self, "terraindata");
    ilS_pushFunc(self->L, "__index", &ter_index);

    ilS_typeTable(self->L, "terraindata");

    ilS_endTable(self, l, "terraindata");

}
