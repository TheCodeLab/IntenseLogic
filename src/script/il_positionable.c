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

int il_Common_Positionable_wrap(lua_State* L, il_Common_Positionable* p);

static int pos_index(lua_State* L)
{
    il_Common_Positionable* self = il_Script_getPointer(L, 1, "positionable", NULL);
    il_Common_String k = il_Script_getString(L, 2);

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
        return il_Common_World_wrap(L, self->parent);
    }

    lua_getglobal(L, "positionable");
    lua_pushvalue(L, 2);
    lua_gettable(L, -2);

    return 1;
}

static int pos_newindex(lua_State* L)
{
    il_Common_Positionable* self = il_Script_getPointer(L, 1, "positionable", NULL);
    il_Common_String k = il_Script_getString(L, 2);

    if (luaL_testudata(L, 3, "vector3")) {
        sg_Vector3* v = il_Script_getPointer(L, 3, "vector3", NULL);
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
        sg_Quaternion q = *(sg_Quaternion*)il_Script_getPointer(L, 3, "quaternion", NULL);
        self->rotation = q;
        return 0;
    }
    if (luaL_testudata(L, 3, "world")) {
        self->parent = il_Script_getPointer(L, 3, "world", NULL);
        return 0;
    }
    return 0;
}

int il_Common_Positionable_wrap(lua_State* L, il_Common_Positionable* p)
{
    return il_Script_createMakeLight(L, p, "positionable");
}

static int pos_create(lua_State* L)
{
    il_Graphics_World * world = il_Script_getPointer(L, 1, "world", NULL);
    il_Common_Positionable* p = il_Common_Positionable_new(world->world);

    return il_Common_Positionable_wrap(L, p);
}

void il_Common_Positionable_luaGlobals(il_Script_Script* self, void * ctx)
{
    (void)ctx;

    const luaL_Reg l[] = {
        {"create",    &pos_create},
        {"getType",   &il_Script_typeGetter},
        {"isA",       &il_Script_isA},

        {NULL,        NULL}
    };

    il_Script_startTable(self, l);

    il_Script_startMetatable(self, "positionable");
    il_Script_pushFunc(self->L, "__index", &pos_index);
    il_Script_pushFunc(self->L, "__newindex", &pos_newindex);

    il_Script_typeTable(self->L, "positionable");

    il_Script_endTable(self, l, "positionable");
}
