#ifndef IL_GRAPHICS_TERRAIN_H
#define IL_GRAPHICS_TERRAIN_H

#include "common/terrain.h"
#include "common/positionable.h"

typedef struct ilG_terrain ilG_terrain;

ilG_terrain* ilG_terrain_new(il_terrain*, il_positionable*);

#endif
