#ifndef IL_GRAPHICS_GRAPHICS_H
#define IL_GRAPHICS_GRAPHICS_H

#include "common/event.h"
#include "asset/node.h"

extern il_base *ilG_shaders_dir;
extern const ilA_dir *ilG_shaders_iface;

void ilG_shaders_addPath(const char *path);

#endif
