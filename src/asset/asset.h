#ifndef IL_ASSET_ASSET_H
#define IL_ASSET_ASSET_H

#include <stdio.h>
#include "common/string.h"

void il_Asset_init();

typedef struct il_Asset_Asset il_Asset_Asset;

void il_Asset_setWriteDir(il_Common_String path);

void il_Asset_registerReadDir(il_Common_String path, int priority);

il_Asset_Asset* il_Asset_open(il_Common_String path);

FILE* il_Asset_getHandle(il_Asset_Asset* asset);

void il_Asset_close(il_Asset_Asset* asset);

int il_Asset_delete(il_Asset_Asset* asset);

#endif
