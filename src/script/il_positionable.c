#include "common/positionable.h"

#include "script/script.h"
#include "script/il.h"
#include "common/string.h"
#include "common/vector.h"
#include "common/matrix.h"
#include "graphics/world.h"
#include "interface.h"

extern int sg_Vector3_wrap(lua_State* L, sg_Vector3 v);
int sg_Quaternion_wrap(lua_State* L, sg_Quaternion q);

int il_positionable_wrap(lua_State* L, il_positionable* p);

static int pos_index(lua_State* L)
{
    il_positionable* self = ilS_getPointer(L, 1, "positionable", NULL);
    il_string k = ilS_getString(L, 2);

    if (il_strcmp(k, il_l("position"))) {
        return sg_Vector3_wrap(L, self->position);
    }
    if (il_strcmp(k, il_l("size"))) {
        return sg_Vector3_wrap(L, self->size);
    }
    if (il_strcmp(k, il_l("velocity"))) {
        return sg_Vector3_wrap(L, self->velocity);
    }
    if (il_strcmp(k, il_l("rotation"))) {
        return sg_Quaternion_wrap(L, self->rotation);
    }
    if (il_strcmp(k, il_l("parent"))) {
        return il_world_wrap(L, self->parent);
    }

    lua_getglobal(L, "positionable");
    lua_pushvalue(L, 2);
    lua_gettable(L, -2);

    return 1;
}

static int pos_newindex(lua_State* L)
{
    il_positionable* self = ilS_getPointer(L, 1, "positionable", NULL);
    il_string k = ilS_getString(L, 2);

    if (luaL_testudata(L, 3, "vector3")) {
        sg_Vector3* v = ilS_getPointer(L, 3, "vector3", NULL);
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
        return 0;
    }
    if (luaL_testudata(L, 3, "quaternion")) {
        sg_Quaternion q = *(sg_Quaternion*)ilS_getPointer(L, 3, "quaternion", NULL);
        self->rotation = q;
        return 0;
    }
    if (luaL_testudata(L, 3, "world")) {
        self->parent = ilS_getPointer(L, 3, "world", NULL);
        return 0;
    }
    return 0;
}

int il_positionable_wrap(lua_State* L, il_positionable* p)
{
    return ilS_createMakeLight(L, p, "positionable");
}

static int pos_create(lua_State* L)
{
    ilG_world * world = ilS_getPointer(L, 1, "world", NULL);
    il_positionable* p = il_positionable_new(world->world);

    return il_positionable_wrap(L, p);
}

void il_positionable_luaGlobals(ilS_script* self, void * ctx)
{
    (void)ctx;

    const luaL_Reg l[] = {
        {"create",    &pos_create},
        {"getType",   &ilS_typeGetter},
        {"isA",       &ilS_isA},

        {NULL,        NULL}
    };

    ilS_startTable(self, l);

    ilS_startMetatable(self, "positionable");
    ilS_pushFunc(self->L, "__index", &pos_index);
    ilS_pushFunc(self->L, "__newindex", &pos_newindex);

    ilS_typeTable(self->L, "positionable");

    ilS_endTable(self, l, "positionable");
}
