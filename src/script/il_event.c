#include "common/event.h"

#include <sys/time.h>
#include <math.h>

#include "script/script.h"
#include "script/il.h"

int il_Event_wrap(lua_State* L, const il_Event_Event* e) {
  return il_Script_createMakeHeavy(L, sizeof(il_Event_Event) + e->size, e, "event");
}

struct ctx {
  int ref;
  lua_State* L;
};
static void cb(il_Event_Event* self, void * rawctx) {
  int ref = ((struct ctx*)rawctx)->ref;
  lua_State* L = ((struct ctx*)rawctx)->L;
  
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
  
  il_Event_wrap(L, self);
  lua_pcall(L, 1, 0, 0);
}
static int register_cb(lua_State* L) {
  if (!lua_isnumber(L, 1)) return luaL_argerror(L, 1, "Expected number between 0 and 65536");
  if (!lua_isfunction(L, 2)) return luaL_argerror(L, 2, "Expected function");
  if (lua_gettop(L) > 2) return luaL_argerror(L, 3, "Extraneous argument");
  
  struct ctx* ptr = calloc(1, sizeof(struct ctx));
  ptr->ref = LUA_NOREF;
  ptr->ref = luaL_ref(L, LUA_REGISTRYINDEX);
  if (ptr->ref == LUA_NOREF) return luaL_error(L, "Unable to make reference to callback!");
  ptr->L = L;
  
  unsigned short id;
  id = (unsigned short)il_Script_getNumber(L, 1);
  
  il_Event_register(id, &cb, ptr);
  
  return 0;
}

static int push(lua_State* L) {
  il_Event_Event* self = il_Script_getPointer(L, 1, "event", NULL);
  il_Event_push(self);
  return 0;
}

static int timer(lua_State* L) {
  il_Event_Event* self = il_Script_getPointer(L, 1, "event", NULL);
  struct timeval* interval = malloc(sizeof(struct timeval));
  double n = il_Script_getNumber(L, 2);
  interval->tv_sec = (long long)floor(n/1000000.0);
  interval->tv_usec = (suseconds_t)n;
  il_Event_timer(self, interval);
  return 0;
}

static int create(lua_State* L) {
  il_Event_Event e;
  memset(&e, 0, sizeof(il_Event_Event));
  double n = il_Script_getNumber(L, 1);
  if (n < 0 || n > 65536) return luaL_argerror(L, 1, "Expected number between 0 and 65536");
  e.eventid = (unsigned short)n;
  
  return il_Event_wrap(L, &e);
}

void il_Event_luaGlobals(il_Script_Script* self, void * ctx) {

  const luaL_Reg l[] = {
    {"create",    &create               },
    {"getType",   &il_Script_typeGetter },
    {"isA",       &il_Script_isA        },
    
    {"register",  &register_cb          },
    {"push",      &push                 },
    {"timer",     &timer                },
    
    {NULL,        NULL                  }
  };

  il_Script_startTable(self, l);

  il_Script_startMetatable(self, "event");
  
  il_Script_typeTable(self->L, "event");
  
  il_Script_endTable(self, l, "event");
}
