#include "script.h"

#include <string.h>

#include "util/log.h"
#include "util/ilstring.h"

int ilS_script_wrap(lua_State* L, ilS_script* s);

static int print(lua_State* L)
{
    lua_Debug ar;
    int res = lua_getstack(L, 2, &ar); // level 2 so we skip this function, and
    // the lua version that converts args to a string for us
    if (res != 1) return -1;
    res = lua_getinfo(L, "nSl", &ar);
    if (!res) return -1;

    il_log_real(
        ar.short_src,
        ar.currentline,
        ar.name,
        3,
        "%s",
        lua_tostring(L, -1)
    );

    return 0;
}

// Blatantly stolen from the lua standalone interpreter
static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getglobal(L, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}

ilS_script * ilS_new()
{
    ilS_script* s = calloc(1, sizeof(ilS_script));

    return s;
}

int ilS_fromAsset(ilS_script* self, ilA_asset * asset)
{
    if (!self || !asset) return -1;
    self->filename = il_StoC(ilA_getPath(asset));
    return ilS_fromSource(self, ilA_readContents(asset));
}

struct reader_ctx {
    int loaded;
    il_string *source;
};
static const char * reader(lua_State* L, void * data, size_t * size)
{
    (void)L;
    struct reader_ctx * ctx = (struct reader_ctx*)data;
    if (ctx->loaded) return NULL;
    *size = ctx->source->length;
    ctx->loaded = 1;
    return ctx->source->data;
}

int ilS_fromSource(ilS_script* self, il_string *source)
{
    if (!self || !source) return -1;
    self->source = il_string_ref(source);
    struct reader_ctx data;
    data.loaded = 0;
    data.source = il_string_ref(source);
    char * chunkname = NULL;
    if (self->filename) {
        chunkname = malloc(strlen(self->filename) + 2);
        sprintf(chunkname, "@%s", self->filename);
    }

    if (!self->L)
        self->L = luaL_newstate();
    luaL_openlibs(self->L);

    lua_pushcfunction(self->L, &print);
    lua_setglobal(self->L, "print");

    lua_pushcfunction(self->L, &traceback);
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
    return ilS_fromAsset(self, ilA_open(il_string_new(filename, strlen(filename))));
}

int ilS_run(ilS_script* self)
{
    self->running = 1;
    il_log("Running script %s", self->filename);
    int res = lua_pcall(self->L, 0, 0, self->ehandler);
    if (res) {
        self->err = lua_tolstring(self->L, -1, &self->errlen);
        self->running = -1;
        return -1;
    }
    return 0;
}

