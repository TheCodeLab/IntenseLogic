#include "common/event.h"

#include <sys/time.h>
#include <math.h>

#include "script/script.h"
#include "script/il.h"
#include "common/input.h"
#include "common/base.h"

int ilE_wrap(lua_State* L, const ilE_event* e)
{
    return ilS_createMakeHeavy(L, sizeof(ilE_event) + e->size, e, "event");
}

static const char* idtostring(unsigned short num)
{
    switch(num) {
#define add_id(id, name) if (id == num) return name
        add_id( IL_INPUT_KEYDOWN,    "keydown"   );
        add_id( IL_INPUT_KEYUP,      "keyup"     );
        add_id( IL_INPUT_MOUSEDOWN,  "mousedown" );
        add_id( IL_INPUT_MOUSEUP,    "mouseup"   );
        add_id( IL_INPUT_MOUSEMOVE,  "mousemove" );
        add_id( IL_INPUT_MOUSEWHEEL, "mousewheel");
        add_id( IL_BASE_TICK,        "tick"      );
        add_id( IL_BASE_STARTUP,     "startup"   );
        add_id( IL_BASE_SHUTDOWN,    "shutdown"  );
#undef add_id
    default:
        return NULL;
    }
}
static int getidstring(lua_State* L)
{
    unsigned n = luaL_checkunsigned(L, 1);
    if (n > 65535) return luaL_argerror(L, 1, "Expected number under 65535");
    const char * s = idtostring((unsigned short)n);
    if (s) lua_pushlstring(L, s, strlen(s));
    else lua_pushnil(L);
    return 1;
}

static unsigned short stringtoid(const char* s)
{
#define add_id(id, name) if (strcmp(s, name) == 0) return id
    add_id( IL_INPUT_KEYDOWN,    "keydown"   );
    add_id( IL_INPUT_KEYUP,      "keyup"     );
    add_id( IL_INPUT_MOUSEDOWN,  "mousedown" );
    add_id( IL_INPUT_MOUSEUP,    "mouseup"   );
    add_id( IL_INPUT_MOUSEMOVE,  "mousemove" );
    add_id( IL_INPUT_MOUSEWHEEL, "mousewheel");
    add_id( IL_BASE_TICK,        "tick"      );
    add_id( IL_BASE_STARTUP,     "startup"   );
    add_id( IL_BASE_SHUTDOWN,    "shutdown"  );
#undef add_id
    return -1;
}

struct ctx {
    int ref;
    lua_State* L;
};
static void cb(ilE_event* self, void * rawctx)
{
    int ref = ((struct ctx*)rawctx)->ref;
    lua_State* L = ((struct ctx*)rawctx)->L;

    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

    ilE_wrap(L, self);
    lua_pcall(L, 1, 0, 0);
}
static int register_cb(lua_State* L)
{
    if (!lua_isnumber(L, 1) && !lua_isstring(L, 1))
        return luaL_argerror(L, 1, "Expected string or number between 0 and 65536");
    if (!lua_isfunction(L, 2)) return luaL_argerror(L, 2, "Expected function");
    if (lua_gettop(L) > 2) return luaL_argerror(L, 3, "Extraneous argument");

    struct ctx* ptr = calloc(1, sizeof(struct ctx));
    ptr->ref = LUA_NOREF;
    ptr->ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (ptr->ref == LUA_NOREF) return luaL_error(L, "Unable to make reference to callback!");
    ptr->L = L;

    unsigned short id;
    if (lua_isnumber(L, 1)) {
        double n = ilS_getNumber(L, 1);
        if (n < 0 || n > 65535) return luaL_argerror(L, 1, "Expected number between 0 and 65535");
        id = (unsigned short)n;
    } else
        id = stringtoid(lua_tostring(L, 1));

    ilE_register(id, &cb, ptr);

    return 0;
}

static int push(lua_State* L)
{
    ilE_event* self = ilS_getPointer(L, 1, "event", NULL);
    ilE_push(self);
    return 0;
}

static int timer(lua_State* L)
{
    ilE_event* self = ilS_getPointer(L, 1, "event", NULL);
    struct timeval* interval = malloc(sizeof(struct timeval));
    double n = luaL_checknumber(L, 2);
    interval->tv_sec = (long long)floor(n/1000000.0);
    interval->tv_usec = n;
    ilE_timer(self, interval);
    return 0;
}

static int getdata(lua_State* L)
{
    ilE_event* self = ilS_getPointer(L, 1, "event", NULL);
    void * data = &self->data;
    switch (self->eventid) {
    case IL_INPUT_KEYDOWN:
    case IL_INPUT_KEYUP:
    case IL_INPUT_MOUSEDOWN:
    case IL_INPUT_MOUSEUP:
        if (self->size < sizeof(int)) luaL_argerror(L, 1, "Malformed event");
        lua_pushinteger(L, *(int*)data);
        return 1;
    case IL_INPUT_MOUSEMOVE:
        if (self->size < sizeof(ilI_mouseMove)) luaL_argerror(L, 1, "Malformed event");
        lua_pushinteger(L, ((ilI_mouseMove*)data)->x);
        lua_pushinteger(L, ((ilI_mouseMove*)data)->y);
        return 2;
    case IL_INPUT_MOUSEWHEEL:
        if (self->size < sizeof(ilI_mouseWheel)) luaL_argerror(L, 1, "Malformed event");
        lua_pushinteger(L, ((ilI_mouseWheel*)data)->y);
        lua_pushinteger(L, ((ilI_mouseWheel*)data)->x);
        return 2;
    case IL_BASE_TICK:
    case IL_BASE_STARTUP:
    case IL_BASE_SHUTDOWN:
        lua_pushnil(L);
        return 1;
    default:
        return luaL_argerror(L, 1, "Unrecognised event type");
    }
}

static int getid(lua_State* L)
{
    ilE_event* self = ilS_getPointer(L, 1, "event", NULL);
    lua_pushinteger(L, self->eventid);
    return 1;
}

static int create(lua_State* L)
{
    ilE_event e;
    memset(&e, 0, sizeof(ilE_event));
    unsigned n = luaL_checkunsigned(L, 1);
    if (n > 65535) return luaL_argerror(L, 1, "Expected number between 0 and 65535");
    e.eventid = (unsigned short)n;

    return ilE_wrap(L, &e);
}

void ilE_luaGlobals(ilS_script* self, void * ctx)
{
    (void)ctx;

    const luaL_Reg l[] = {
        {"create",    &create               },
        {"getType",   &ilS_typeGetter },
        {"isA",       &ilS_isA        },

        {"register",  &register_cb          },
        {"push",      &push                 },
        {"timer",     &timer                },
        {"getData",   &getdata              },
        {"getId",     &getid                },
        {"getIdName", &getidstring          },

        {NULL,        NULL                  }
    };

    ilS_startTable(self, l);

    ilS_startMetatable(self, "event");

    ilS_typeTable(self->L, "event");

    ilS_endTable(self, l, "event");
}
