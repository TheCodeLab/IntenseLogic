#include "common/quaternion.h"

#include "script/script.h"
#include "script/il.h"

int sg_Quaternion_wrap(lua_State* L, sg_Quaternion q);

static int quat_index(lua_State* L)
{
    sg_Quaternion* q = ilS_getPointer(L, 1, "quaternion", NULL);
    il_String k = ilS_getString(L, 2);

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

static int quat_newindex(lua_State* L)
{
    sg_Quaternion* q = ilS_getPointer(L, 1, "quaternion", NULL);
    il_String k = ilS_getString(L, 2);
    double v = luaL_checknumber(L, 3);

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

static int quat_mul(lua_State* L)
{
    sg_Quaternion* a = ilS_getPointer(L, 1, "quaternion", NULL);
    sg_Quaternion* b = ilS_getPointer(L, 2, "quaternion", NULL);
    sg_Quaternion res = sg_Quaternion_mul(*a, *b);
    return sg_Quaternion_wrap(L, res);
}

static int quat_tostring(lua_State* L)
{
    sg_Quaternion* q = (sg_Quaternion*)ilS_getPointer(L, 1, "quaternion", NULL);
    lua_pushfstring(L, "(%f, %f, %f), %f",
                    (lua_Number)q->x, (lua_Number)q->y, (lua_Number)q->z, (lua_Number)q->w);
    return 1;
}

static int quat_fromAxisAngle(lua_State* L)
{
    sg_Vector3 v;
    float a;
    if (luaL_testudata(L, 1, "vector3")) {
        v = *(sg_Vector3*)ilS_getPointer(L, 1, "vector3", NULL);
        a = luaL_checkinteger(L, 2);
    } else {
        v.x = luaL_checkinteger(L, 1);
        v.y = luaL_checkinteger(L, 2);
        v.z = luaL_checkinteger(L, 3);
        a = luaL_checkinteger(L, 4);
    }
    sg_Quaternion q = sg_Quaternion_fromAxisAngle(v, a);
    return sg_Quaternion_wrap(L, q);
}

static int quat_fromYPR(lua_State* L)
{
    float y, p, r;
    y = luaL_checknumber(L, 1);
    p = luaL_checknumber(L, 2);
    r = luaL_checknumber(L, 3);
    sg_Quaternion q = sg_Quaternion_fromYPR(y, p, r);
    return sg_Quaternion_wrap(L, q);
}

int sg_Quaternion_wrap(lua_State* L, sg_Quaternion q)
{
    return ilS_createMakeHeavy(L, sizeof(sg_Quaternion), &q, "quaternion");
}

static int quat_create(lua_State* L)
{
    sg_Quaternion q;
    q.x = luaL_optnumber(L, 1, 0.0);
    q.y = luaL_optnumber(L, 2, 0.0);
    q.z = luaL_optnumber(L, 3, 0.0);
    q.w = luaL_optnumber(L, 4, 1.0);
    return sg_Quaternion_wrap(L, q);
}

void sg_Quaternion_luaGlobals(ilS_script* self, void * ctx)
{
    (void)ctx;

    const luaL_Reg l[] = {
        {"create",        &quat_create          },
        {"getType",       &ilS_typeGetter },
        {"isA",           &ilS_isA        },

        {"fromAxisAngle", &quat_fromAxisAngle   },
        {"fromYPR",       &quat_fromYPR         },

        {NULL,            NULL                  }
    };

    ilS_startTable(self, l);

    ilS_startMetatable(self, "quaternion");
    ilS_pushFunc(self->L, "__index", &quat_index);
    ilS_pushFunc(self->L, "__newindex", &quat_newindex);
    ilS_pushFunc(self->L, "__mul", &quat_mul);
    ilS_pushFunc(self->L, "__tostring", &quat_tostring);

    ilS_typeTable(self->L, "quaternion");

    ilS_endTable(self, l, "quaternion");
}
