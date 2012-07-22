#ifndef IL_SCRIPT_H
#define IL_SCRIPT_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

typedef struct {
    int success;
    char *error;
} il_Script_status;

void il_Script_init();
il_Script_status *il_Script_loadfile(char *filename);

#endif
