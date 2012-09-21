#ifndef IL_GRAPHICS_TERRAIN_H
#define IL_GRAPHICS_TERRAIN_H

#include "common/terrain.h"
#include "common/positionable.h"

typedef struct il_Graphics_Terrain il_Graphics_Terrain;

il_Graphics_Terrain* il_Graphics_Terrain_new(il_Common_Terrain*, il_Common_Positionable*);

#endif
