#include "graphics/drawable3d.h"
#include "graphics/shape.h"
#include "graphics/terrain.h"

#include <string.h>

#include "script/script.h"
#include "script/il.h"
#include "interface.h"
#include "common/terrain.h"
#include "graphics/glutil.h"

////////////////////////////////////////////////////////////////////////////////
// Drawable

int ilG_drawable3d_wrap(lua_State* L, ilG_drawable3d* self)
{
    return ilS_createMakeLight(L, self, "drawable");
}

static int drawable_index(lua_State* L)
{
    ilG_drawable3d* self =
        ilS_getChildT(L, 1, NULL, lua_upvalueindex(2));

    const char* k = luaL_checkstring(L, 2);

    if (strcmp(k, "type")) {
        lua_pushinteger(L, self->type);
        return 1;
    }
    if (strcmp(k, "positionable")) {
        il_positionable_wrap(L, self->positionable);
        return 1;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Terrain

struct ilG_terrain {
    ilG_drawable3d drawable;
    il_terrain * terrain;
    GLuint *buf;
    GLuint program;
    void *draw_ctx;
    void (*draw)(ilG_terrain*, void*, const ilG_camera*,
                 const struct timeval*);
};

int ilG_terrain_wrap(lua_State* L, ilG_terrain* self)
{
    return ilS_createMakeLight(L, self, "terrain");
}

static int terrain_create(lua_State* L)
{
    il_terrain* ter = ilS_getPointer(L, 1, "terraindata", NULL);
    il_positionable* pos = ilS_getPointer(L, 2, "positionable", NULL);

    ilG_terrain* self = ilG_terrain_new(ter, pos);

    return ilG_terrain_wrap(L, self);
}

static int terrain_index(lua_State* L)
{
    ilG_terrain* self =
        ilS_getChildT(L, 1, NULL, lua_upvalueindex(2));
    const char * k = luaL_checkstring(L, 2);

    if (strcmp(k, "data")) {
        return il_terrain_wrap(L, self->terrain);
    }

    return drawable_index(L);
}

////////////////////////////////////////////////////////////////////////////////
// Shape

struct ilG_shape {
    ilG_drawable3d drawable;
    int type;
    GLuint vbo;
    GLuint ibo;
    GLuint vao;
    GLuint program;
    GLuint texture;
    GLenum mode;
    GLsizei count;
};

int ilG_shape_wrap(lua_State* L, ilG_shape* self)
{
    return ilS_createMakeLight(L, self, "shape");
}

static const char * const shape_lst[] = {"box", "cylinder", "sphere", "plane"};

static int shape_create(lua_State* L)
{
    int type = luaL_checkoption(L, 1, "box", shape_lst);
    il_positionable* pos;
    if (!lua_isnone(L, 2))
        pos = ilS_getPointer(L, 1, "positionable", NULL);
    else
        pos = il_positionable_new(ilS_getPointer(L, 1, "world", NULL));

    ilG_shape* self = ilG_shape_new(pos, type);

    return ilG_shape_wrap(L, self);
}

static int shape_index(lua_State* L)
{
    ilG_shape* self = ilS_getPointer(L, 1, "shape", NULL);
    const char * k = luaL_checkstring(L, 2);

    if (strcmp(k, "type")) {
        const char * const t = shape_lst[self->type];
        lua_pushlstring(L, t, strlen(t));
        return 1;
    }

    return drawable_index(L);
}

////////////////////////////////////////////////////////////////////////////////

void ilG_drawable3d_luaGlobals(ilS_script* self, void * ctx)
{
    (void)ctx;

    //////////////
    // Drawable

    const luaL_Reg l[] = {
        {"getType", &ilS_typeGetter},
        {"isA", &ilS_isA},

        {NULL, NULL}
    };

    ilS_startTable(self, l);

    ilS_startMetatable(self, "drawable");
    ilS_pushFunc(self->L, "__index", &drawable_index);

    ilS_typeTable(self->L, "drawable");

    ilS_endTable(self, l, "drawable");

    /////////////
    // Terrain

    const luaL_Reg terrain[] = {
        {"create", &terrain_create},

        {NULL, NULL}
    };

    ilS_startTable(self, terrain);

    ilS_startMetatable(self, "terrain");
    ilS_pushFunc(self->L, "__index", &terrain_index);

    ilS_typeTable(self->L, "terrain", "drawable");

    luaL_setfuncs(self->L, l, 2); // put the "parent" functions into this table

    // continue as usual

    ilS_startMetatable(self, "terrain");

    ilS_typeTable(self->L, "terrain", "drawable");

    ilS_endTable(self, l, "terrain");

    ///////////
    // Shape

    const luaL_Reg shape[] = {
        {"create", &shape_create},

        {NULL, NULL}
    };

    ilS_startTable(self, shape);

    ilS_startMetatable(self, "shape");
    ilS_pushFunc(self->L, "__index", &shape_index);

    ilS_typeTable(self->L, "shape", "drawable");

    luaL_setfuncs(self->L, l, 2); // put the "parent" functions into this table

    // continue as usual

    ilS_startMetatable(self, "shape");

    ilS_typeTable(self->L, "shape", "drawable");

    ilS_endTable(self, l, "shape");

}
