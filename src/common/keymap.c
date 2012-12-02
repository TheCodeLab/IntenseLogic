#ifndef IL_COMMON_KEYMAP_H
#define IL_COMMON_KEYMAP_H

#define IL_CONFIG_TYPE il_keymap
#define IL_CONFIG_DEFS "common/keymap.defs"

#include "common/config.inc"

#undef IL_CONFIG_TYPE
#undef IL_CONFIG_DEFS

#include <stdlib.h>

int il_keymap_getkey(const char* key)
{
    if (strlen(key) == 1)
        return (int)key[0];
    return atoi(key);
}

#endif
