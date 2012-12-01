#ifndef IL_COMMON_KEYMAP_H
#define IL_COMMON_KEYMAP_H

#define IL_CONFIG_TYPE il_keymap
#define IL_CONFIG_DEFS "common/keymap.defs"

#include "common/config.h"

#undef IL_CONFIG_TYPE
#undef IL_CONFIG_DEFS

int il_keymap_getkey(const char* key);

#endif
