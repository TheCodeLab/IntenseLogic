#ifndef IL_GRAPHICS_GRAPHICS_H
#define IL_GRAPHICS_GRAPHICS_H

#include "util/event.h"
#include "asset/node.h"

extern ilA_fs ilG_shaders;

int il_load_ilgraphics();
void ilG_shaders_addPath(const char *path);
void ilG_quit();

#endif
