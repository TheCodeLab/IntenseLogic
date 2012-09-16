#ifndef IL_COMMON_KEYMAP_H
#define IL_COMMON_KEYMAP_H

#define IL_CONFIG_TYPE il_Common_Keymap
#define IL_CONFIG_DEFS "common/keymap.defs"

#include "common/config.h"

#undef IL_CONFIG_TYPE
#undef IL_CONFIG_DEFS

int il_Common_Keymap_getkey(const char* key);

#endif
