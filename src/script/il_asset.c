#include "asset/asset.h"

#include <string.h>

#include "script/script.h"
#include "script/il.h"

int ilA_wrap(lua_State* L, ilA_asset* asset)
{
    return ilS_createMakeLight(L, asset, "asset");
}

static int asset_index(lua_State* L)
{
    ilA_asset* self = ilS_getPointer(L, 1, "asset", NULL);
    il_String key = ilS_getString(L, 2);

    if (il_strcmp(key, il_l("path")) == 0) {
        il_String res = ilA_getPath(self);
        lua_pushlstring(L, res.data, res.length);
        return 1;
    }
    if (il_strcmp(key, il_l("data")) == 0) {
        il_String res = ilA_readContents(self);
        lua_pushlstring(L, res.data, res.length);
        return 1;
    }

    return 0;
}

static int setwritedir(lua_State* L)
{
    il_String path = ilS_getString(L, 1);
    ilA_setWriteDir(path);
    return 0;
}

static int registerreaddir(lua_State* L)
{
    il_String path = ilS_getString(L, 1);
    int priority = luaL_optinteger(L, 2, 0);
    ilA_registerReadDir(path, priority);
    return 0;
}

static int asset_close(lua_State* L)
{
    ilA_asset* self = ilS_getPointer(L, 1, "asset", NULL);
    ilA_close(self);
    return 0;
}

static int asset_delete(lua_State* L)
{
    ilA_asset* self = ilS_getPointer(L, 1, "asset", NULL);
    ilA_delete(self);
    return 0;
}

static int create(lua_State* L)
{
    il_String path = ilS_getString(L, 1);
    ilA_asset* asset = ilA_open(path);
    return ilA_wrap(L, asset);
}

void ilA_luaGlobals(ilS_script* self, void * ctx)
{
    (void)ctx;

    luaL_Reg l[] = {
        {"create",          &create               },
        {"getType",         &ilS_typeGetter },
        {"isA",             &ilS_isA        },

        {"setWriteDir",     &setwritedir          },
        {"registerReadDir", &registerreaddir      },
        {"close",           &asset_close          },
        {"delete",          &asset_delete         },

        {NULL,              NULL                  }
    };

    ilS_startTable(self, l);

    ilS_startMetatable(self, "asset");
    ilS_pushFunc(self->L, "__index", &asset_index);

    ilS_typeTable(self->L, "asset");

    ilS_endTable(self, l, "asset");
}
