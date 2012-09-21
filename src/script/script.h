#ifndef IL_SCRIPT_H
#define IL_SCRIPT_H

#include <stdlib.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct {
    int success;
    size_t errlen;
    const char *error;
} il_Script_Status;

void il_Script_init();
il_Script_Status il_Script_loadfile(char *filename);

#endif
