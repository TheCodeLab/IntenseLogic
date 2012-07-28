#include "script.h"

void il_Script_init(){
}

il_Script_Status il_Script_loadfile(char *filename){
    il_Script_Status status;
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0)){
        status.success = 0;
        status.error   = lua_tolstring(L, -1, &status.errlen);
    } else{
        status.success = 1;
    }

    lua_close(L);

    return status;
}
