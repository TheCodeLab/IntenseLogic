#include "common/world.h"
#include "graphics/world.h"

#include <string.h>

#include "script/script.h"
#include "script/il.h"
#include "interface.h"

int ilG_world_wrap(lua_State* L, ilG_world * w)
{
    return ilS_createMakeLight(L, w, "world");
}

int il_world_wrap(lua_State* L, il_world * self)
{
    ilG_world* w = il_world_getGraphicsWorld(self);
    if (w) return ilG_world_wrap(L, w);
    return ilG_world_wrap(L, ilG_world_new_world(self));
}

static int world_create(lua_State* L)
{
    ilG_world* w = ilG_world_new();
    return ilG_world_wrap(L, w);
}

static int world_index(lua_State* L)
{
    ilG_world* self = ilS_getPointer(L, 1, "world", NULL);
    const char * k = luaL_checkstring(L, 1);

    if (strcmp(k, "camera")) {
        return ilG_camera_wrap(L, self->camera);
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
