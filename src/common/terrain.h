#ifndef IL_COMMON_TERRAIN_H
#define IL_COMMON_TERRAIN_H

#include "common/matrix.h"

typedef struct il_Common_Terrain il_Common_Terrain;

il_Common_Terrain * il_Common_Terrain_new();

void il_Common_Terrain_getSize(il_Common_Terrain*, int *width, int *height);
double il_Common_Terrain_getPoint(il_Common_Terrain*, unsigned x, unsigned y, double height);
sg_Vector3 il_Common_Terrain_getNormal(il_Common_Terrain*, unsigned x, unsigned y, double z);

int il_Common_Terrain_heightmapFromMemory(il_Common_Terrain*, 
  int width, int height, const float * points);

#endif
