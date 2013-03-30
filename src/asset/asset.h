#ifndef IL_ASSET_ASSET_H
#define IL_ASSET_ASSET_H

#include <stdio.h>
#include "util/ilstring.h"

void ilA_init();

#define IL_ASSET_READFILE(name) \
  (ilA_readContents(ilA_open(il_string_new(name, -1))))

typedef struct ilA_asset ilA_asset;

void ilA_setWriteDir(il_string *path);

void ilA_registerReadDir(il_string *path, int priority);

ilA_asset* ilA_open(il_string *path);

ilA_asset* ilA_ref(void*);

void ilA_unref(void*);

il_string *ilA_getPath(ilA_asset*);

FILE* ilA_getHandle(ilA_asset* asset, const char *flags);

il_string *ilA_readContents(ilA_asset* asset);

void ilA_close(ilA_asset* asset);

int ilA_delete(ilA_asset* asset);

#endif
