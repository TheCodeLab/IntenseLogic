#include "common/world.h"

#include <string.h>

#include "script/script.h"
#include "script/il.h"
#include "interface.h"

int il_world_wrap(lua_State* L, il_world * w)
{
    return ilS_createMakeLight(L, w, "world");
}

static int world_create(lua_State* L)
{
    il_world* w = il_world_new();
    return il_world_wrap(L, w);
}

static int world_index(lua_State* L)
{
    il_world* self = ilS_getPointer(L, 1, "world", NULL);
    const char * k = luaL_optstring(L, 2, NULL);

    if (strcmp(k, "id")) {
        lua_pushinteger(L, self->id);
        return 1;
    }

    return 0;
}

void ilG_world_luaGlobals(ilS_script* self, void * ctx)
{
    (void)ctx;

    const luaL_Reg l[] = {
        {"create", &world_create},
        {"getType", &ilS_typeGetter},
        {"isA", &ilS_isA},

        {NULL, NULL}
    };

    ilS_startTable(self, l);

    ilS_startMetatable(self, "world");
    ilS_pushFunc(self->L, "__index", &world_index);

    ilS_typeTable(self->L, "world");

    ilS_endTable(self, l, "world");
}
