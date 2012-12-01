#include "common/vector.h"

#include "script/il.h"
#include "script/script.h"
#include "script/interface.h"

#define vec2_wrap sg_Vector2_wrap
#define vec3_wrap sg_Vector3_wrap
#define vec4_wrap sg_Vector4_wrap

#define gen_fun(name, op, d) \
    static int vec##d##_##name(lua_State* L) { \
        sg_Vector##d* a = (sg_Vector##d*)ilS_getPointer(L, 1, "vector" #d, NULL); \
        sg_Vector##d res; \
        if (lua_isnumber(L, 2)) { \
            float b = (float)lua_tonumber(L, 2); \
            res = sg_Vector##d##_##name##_f(*a, b); \
        } else { \
            sg_Vector##d* b = (sg_Vector##d*)ilS_getPointer(L, 2, "vector" #d, NULL); \
            res = sg_Vector##d##_##name(*a, *b); \
        } \
        return vec##d##_wrap(L, res); \
        \
    }

#define simple_op(name, op) \
    gen_fun(name, op, 2) \
gen_fun(name, op, 3) \
gen_fun(name, op, 4)

simple_op(add, +)
simple_op(sub, -)
simple_op(mul, *)
simple_op(div, /)

#undef gen_fun
#undef simple_op

static int vec2_tostring(lua_State* L)
{
    sg_Vector2* v = (sg_Vector2*)ilS_getPointer(L, 1, "vector2", NULL);
    lua_pushfstring(L, "%f, %f", (lua_Number)v->x, (lua_Number)v->y);
    return 1;
}

static int vec3_tostring(lua_State* L)
{
    sg_Vector3* v = (sg_Vector3*)ilS_getPointer(L, 1, "vector3", NULL);
    lua_pushfstring(L, "%f, %f, %f", (lua_Number)v->x, (lua_Number)v->y, (lua_Number)v->z);
    return 1;
}

static int vec4_tostring(lua_State* L)
{
    sg_Vector4* v = (sg_Vector4*)ilS_getPointer(L, 1, "vector4", NULL);
    lua_pushfstring(L, "%f, %f, %f, %f", (lua_Number)v->x, (lua_Number)v->y, (lua_Number)v->z, (lua_Number)v->w);
    return 1;
}

#define d 2
#include "vecd_index.inc"
#undef d
#define d 3
#include "vecd_index.inc"
#undef d
#define d 4
#include "vecd_index.inc"
#undef d

#define vecd_wrap(d) \
    int vec##d##_wrap(lua_State* L, sg_Vector##d v) { \
        return ilS_createMakeHeavy(L, sizeof(sg_Vector##d), &v, "vector" #d); \
    }

vecd_wrap(2)
vecd_wrap(3)
vecd_wrap(4)

#undef mt_fun

#define vecd_dot(d) \
        static int vec##d##_dot(lua_State* L) { \
            sg_Vector##d *a = (sg_Vector##d*)ilS_getPointer(L, 1, "vector" #d, NULL); \
            sg_Vector##d *b = (sg_Vector##d*)ilS_getPointer(L, 2, "vector" #d, NULL); \
            float res = sg_Vector##d##_dot(*a, *b); \
            lua_pushnumber(L, res); \
            return 1; \
        }

vecd_dot(2)
vecd_dot(3)
vecd_dot(4)

#define vecd_cross(d) \
        static int vec##d##_cross(lua_State* L) { \
            sg_Vector##d *a = (sg_Vector##d*)ilS_getPointer(L, 1, "vector" #d, NULL); \
            sg_Vector##d *b = (sg_Vector##d*)ilS_getPointer(L, 2, "vector" #d, NULL); \
            sg_Vector##d res = sg_Vector##d##_cross(*a, *b); \
            return vec##d##_wrap(L, res); \
        }

vecd_cross(3)
vecd_cross(4)

#define d 2
#include "vecd_create.inc"
#undef d
#define d 3
#include "vecd_create.inc"
#undef d
#define d 4
#include "vecd_create.inc"
#undef d

void sg_Vector_luaGlobals(ilS_script* self, void* ctx)
{
    (void)ctx;
#define d 2
#include "vecd_globals.inc"
#undef d
#define d 3
#include "vecd_globals.inc"
#undef d
#define d 4
#include "vecd_globals.inc"
#undef d
}
