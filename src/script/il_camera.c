#include "graphics/camera.h"

#include "script/script.h"
#include "script/il.h"
#include "interface.h"

static int camera_index(lua_State* L)
{
    ilG_camera* self = ilS_getPointer(L, 1, "camera", NULL);
    const char * key = luaL_checkstring(L, 2);

    if (strcmp(key, "positionable") == 0) {
        return il_positionable_wrap(L, self->positionable);
    }
    if (strcmp(key, "movespeed") == 0) {
        return il_Vector3_wrap(L, self->movespeed);
    }
    if (strcmp(key, "sensitivity") == 0) {
        lua_pushnumber(L, self->sensitivity);
        return 1;
    }
    // TODO: bind projection_matrix
    return 0;
}

static int newindex(lua_State* L)
{
    ilG_camera* self = ilS_getPointer(L, 1, "camera", NULL);
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "positionable") == 0) {
        self->positionable = ilS_getPointer(L, 3, "positionable", NULL);
        return 0;
    }
    if (strcmp(key, "movespeed") == 0) {
        self->movespeed = *(il_Vector3*)ilS_getPointer(L, 3, "vector3", NULL);
        return 0;
    }
    if (strcmp(key, "sensitivity") == 0) {
        self->sensitivity = luaL_checknumber(L, 3);
        return 0;
    }
    // TODO: bind projection matrix
    return 0;
}

int ilG_camera_wrap(lua_State* L, ilG_camera* self)
{
    return ilS_createMakeLight(L, self, "camera");
}

static int create(lua_State* L)
{
    il_positionable* pos;
    if (luaL_testudata(L, 1, "positionable")) {
        pos = ilS_getPointer(L, 1, "positionable", NULL);
    } else if (luaL_testudata(L, 1, "world")) {
        pos = il_positionable_new(ilS_getPointer(L, 1, "world", NULL));
    } else {
        // TODO: figure out the current world somehow
        luaL_argerror(L, 1, "Expected positionable or world");
    }
    ilG_camera* cam = ilG_camera_new(pos);
    return ilG_camera_wrap(L, cam);
}

void ilG_camera_luaGlobals(ilS_script* self, void * ctx)
{
    (void)ctx;

    luaL_Reg l[] = {
        {"create", &create},
        {"getType", &ilS_typeGetter},
        {"isA", &ilS_isA},

        {NULL, NULL}
    };

    ilS_startTable(self, l);

    ilS_startMetatable(self, "camera");
    ilS_pushFunc(self->L, "__index", &camera_index);
    ilS_pushFunc(self->L, "__newidnex", &newindex);

    ilS_typeTable(self->L, "camera");

    ilS_endTable(self, l, "camera");
}
