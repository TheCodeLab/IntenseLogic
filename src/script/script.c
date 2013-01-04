#include "script.h"

#include <string.h>

#include "common/log.h"
#include "common/string.h"

int ilS_script_wrap(lua_State* L, ilS_script* s);

static int lualog(lua_State* L, int level, int fun)
{
    int nargs = lua_gettop(L);
    il_string strs[nargs];
    il_string str;
    int i;

    if (nargs < 1) {
        printf("\n");
        return 1;
    }

    for (i = 1; i <= nargs; i++) {
        strs[i-1] = il_fromC(lua_tostring(L, i));
    }
    str = strs[0];
    for (i = 2; i <= nargs; i++) {
        if (!strs[i].length) continue;
        str = il_concat(str, il_l("\t"), strs[i-1]);
    }
    lua_Debug ar;
    int res = lua_getstack(L, fun, &ar);
    if (res != 1) return -1;
    res = lua_getinfo(L, "nSl", &ar);
    if (!res) return -1;

    lua_pushfstring(L, "%s:%d (%s) %s: %s\n",
                    ar.short_src,
                    ar.currentline,
                    ar.name,
                    il_loglevel_tostring(level),
                    il_StoC(str)
                   );

    /*if (level <= (int)il_loglevel)
        printf("%s", lua_tostring(L, -1));*/

    return level <= (int)il_loglevel;
}

static int print(lua_State* L)
{
    return lualog(L, 3, 1);
}

static int luaerror(lua_State* L)
{
    int i;
    lua_Debug ar;

    lua_getstack(L, 2, &ar);
    lua_getinfo(L, "S", &ar);

    const char * s = lua_tostring(L, 1);
    if (strstr(s, ar.short_src)) {
        lua_pop(L, 1);
        s = strchr(s, ' ')+1;
        lua_pushlstring(L, s, strlen(s));
    }
    lua_pushliteral(L, "\nStack trace:");

    for (i = 1; lua_getstack(L, i, &ar); i++) {
        lua_getinfo(L, "nSl", &ar);
        lua_pushfstring(L, "\n\t#%d in %s at %s:%d", i, ar.name, ar.short_src, ar.currentline);
    }
    return lualog(L, 1, 2);
}

ilS_script * ilS_new()
{
    ilS_script* s = calloc(1, sizeof(ilS_script));

    return s;
}

int ilS_fromAsset(ilS_script* self, ilA_asset * asset)
{
    if (!self || !asset) return -1;
    self->filename = il_toC(ilA_getPath(asset));
    return ilS_fromSource(self, ilA_readContents(asset));
}

struct reader_ctx {
    int loaded;
    il_string source;
};
static const char * reader(lua_State* L, void * data, size_t * size)
{
    (void)L;
    struct reader_ctx * ctx = (struct reader_ctx*)data;
    if (ctx->loaded) return NULL;
    *size = ctx->source.length;
    ctx->loaded = 1;
    return ctx->source.data;
}

int ilS_fromSource(ilS_script* self, il_string source)
{
    if (!self || !source.length) return -1;
    self->source = source;
    struct reader_ctx data;
    data.loaded = 0;
    data.source = source;
    char * chunkname;
    if (self->filename) {
        chunkname = malloc(strlen(self->filename) + 2);
        sprintf(chunkname, "@%s", self->filename);
    }

    if (!self->L)
        self->L = luaL_newstate();
    luaL_openlibs(self->L);

    //lua_pushcfunction(self->L, &print);
    //lua_setglobal(self->L, "print");

    lua_pushcfunction(self->L, &luaerror);
    self->ehandler = lua_gettop(self->L);

#if LUA_VERSION_NUM == 502
    int res = lua_load(self->L, &reader, &data, chunkname, NULL);
#else
    int res = lua_load(self->L, &reader, &data, chunkname);
#endif
    if (res) {
        self->err = lua_tolstring(self->L, -1, &self->errlen);
        self->running = -1;
        return -1;
    }
    return 0;
}

int ilS_fromFile(ilS_script* self, const char * filename)
{
    return ilS_fromAsset(self, ilA_open(il_fromC((char*)filename)));
}

int ilS_run(ilS_script* self)
{
    self->running = 1;
    il_log(3, "Running script %s", self->filename);
    int res = lua_pcall(self->L, 0, 0, self->ehandler);
    if (res) {
        self->err = lua_tolstring(self->L, -1, &self->errlen);
        self->running = -1;
        return -1;
    }
    return 0;
}

