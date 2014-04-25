#include "illua.h"

#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "util/loader.h"
#include "util/opt.h"

char *strdup(const char*);

static int print(lua_State* L)
{
    lua_Debug ar;
    int res = lua_getstack(L, 2, &ar); // level 2 so we skip this function, and
    // the lua version that converts args to a string for us
    if (res != 1) return -1;
    res = lua_getinfo(L, "nSl", &ar);
    if (!res) return -1;

    fprintf(stderr, "(%s:%i%s%s) %s\n", ar.short_src, ar.currentline, 
            ar.name? " " : "", ar.name? ar.name : "", lua_tostring(L, -1));

    return 0;
}

// Blatantly stolen from the lua standalone interpreter
static int traceback(lua_State *L) 
{
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

static int iterate_modules(lua_State *L)
{
    void **saveptr = (void**)lua_touserdata(L, 1);
    if (!saveptr) {
        saveptr = (void**)lua_newuserdata(L, sizeof(void*));
        *saveptr = NULL;
        return 1; // first call returns the iterator
    }
    const char *str = il_module_iterate(saveptr);
    if (str) {
        lua_pushlstring(L, str, strlen(str));
        return 1;
    }
    return 0;
}

ilS_script * ilS_new()
{
    ilS_script* self = calloc(1, sizeof(ilS_script));

    self->L = luaL_newstate();
    luaL_openlibs(self->L);

    lua_pushcfunction(self->L, &print);
    lua_setglobal(self->L, "print");

    lua_pushcfunction(self->L, iterate_modules);
    lua_setglobal(self->L, "iterate_modules");

    lua_pushcfunction(self->L, &traceback);
    self->ehandler = lua_gettop(self->L);

    return self;
}

struct reader_ctx {
    int loaded;
    const char *source;
    size_t len;
};
static const char * reader(lua_State* L, void * data, size_t * size)
{
    (void)L;
    struct reader_ctx * ctx = (struct reader_ctx*)data;
    if (ctx->loaded) return NULL;
    *size = ctx->len;
    ctx->loaded = 1;
    return ctx->source;
}

int ilS_fromSource(ilS_script* self, const char *source, size_t len)
{
    struct reader_ctx data;
    data.loaded = 0;
    data.source = source;
    data.len = len;
    char * chunkname = NULL;
    if (self->filename) {
        chunkname = malloc(strlen(self->filename) + 2);
        sprintf(chunkname, "@%s", self->filename);
    }

#if LUA_VERSION_NUM == 502
    int res = lua_load(self->L, &reader, &data, chunkname, NULL);
#else
    int res = lua_load(self->L, &reader, &data, chunkname);
#endif
    if (res) {
        self->err = strdup(lua_tolstring(self->L, -1, &self->errlen));
        self->running = -1;
        return -1;
    }
    return 0;
}

void ilS_free(ilS_script* self)
{
    if (self->filename) {
        free(self->filename);
    }
    if (self->err) {
        free(self->err);
    }
    if (self->L) {
        lua_close(self->L);
    }
    free(self);
}

// borrowed from http://stackoverflow.com/a/4156038/2103698 and converted to C
int ilS_addPath(ilS_script* self, const char* path)
{
    lua_State *L = self->L;
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path"); // get field "path" from table at top of stack (-1)
    size_t cur_size;
    const char *cur_path = lua_tolstring(L, -1, &cur_size); // grab path string from top of stack
    char *newpath = calloc(1, cur_size + 1 + strlen(path) + 7); // string + / + string + /?.lua\0
    sprintf(newpath, "%s;%s/?.lua", cur_path, path);
    lua_pop(L, 1); // get rid of the string on the stack we just pushed on line 5
    lua_pushstring(L, newpath); // push the new one
    free(newpath);
    lua_setfield(L, -2, "path"); // set the field "path" in table at -2 with value at top of stack
    lua_pop(L, 1); // get rid of package table from top of stack
    return 0; // all done!
}

int ilS_fromFile(ilS_script* self, const char * filename)
{
    struct stat s;
    stat(filename, &s);
    char contents[s.st_size + 1];
    FILE *f = fopen(filename, "r");
    if (!f) {
        self->err = strerror(errno);
        return -1;
    }
    int res = fread(contents, 1, s.st_size, f);
    if (res < 0) {
        self->err = strerror(errno);
        return -1;
    }
    contents[res] = 0;
    self->filename = strdup(filename);
    return ilS_fromSource(self, contents, res);
}

int ilS_run(ilS_script* self)
{
    self->running = 1;
    fprintf(stderr, "*** Running script %s\n", self->filename);
    int res = lua_pcall(self->L, 0, 0, self->ehandler);
    if (res) {
        self->err = strdup(lua_tolstring(self->L, -1, &self->errlen));
        self->running = -1;
        return -1;
    }
    return 0;
}

static IL_ARRAY(char*,) scripts;
static IL_ARRAY(char*,) paths;
static char *bootstrap;
static int has_scripts;

void il_configure_illua(il_modopts *opts)
{
    for (unsigned i = 0; i < opts->args.length; i++) {
        il_opt *opt = &opts->args.data[i];
        char *arg = strndup(opt->arg.str, opt->arg.len);
#define option(l) if (il_opts_cmp(opt->name, il_optslice_s(l)))
        option("run") {
            IL_APPEND(scripts, arg);
        }
        option("scripts") {
            IL_APPEND(paths, arg);
            has_scripts = 1;
        }
        option("bootstrap") {
            bootstrap = arg;
        }
    }
}

int il_load_illua()
{
    return 0;
}

static void add_paths(ilS_script *self)
{
    for (unsigned i = 0; i < paths.length; i++) {
        ilS_addPath(self, paths.data[i]);
    }
}

static int run_bootstrap(ilS_script *s)
{
    if (bootstrap) {
        return !ilS_fromFile(s, bootstrap);
    } else {
        for (unsigned i = 0; i < paths.length; i++) {
            char path[strlen(paths.data[i]) + strlen("/bootstrap.lua") + 1];
            sprintf(path, "%s/bootstrap.lua", paths.data[i]);
            if (!access(path, F_OK)) { // returns 0 on success
                if (!ilS_fromFile(s, path)) {
                    return !ilS_run(s);
                }
            }
        }
        fprintf(stderr, "Failed to find bootstrap.lua\n");
        return 0;
    }
}

static int run_script(char *path)
{
    ilS_script *s = ilS_new();

    add_paths(s);
    if (!run_bootstrap(s)) {
        return 0;
    }
    if (ilS_fromFile(s, path)) {
        fprintf(stderr, "Failed to load %s: %s\n", s->filename, s->err);
        ilS_free(s);
        return 0;
    }
    if (ilS_run(s)) {
        fprintf(stderr, "Failed to execute %s: %s\n", s->filename, s->err);
        return 0;
    }
    //ilS_free(s);
    return 1;
}

void il_postload_illua()
{
    if (!has_scripts) { // default script path, unless one is specified
        IL_APPEND(paths, "script");
    }

    for (unsigned i = 0; i < scripts.length; i++) {
        run_script(scripts.data[i]);
        free(scripts.data[i]);
    }
    IL_FREE(scripts);
}

