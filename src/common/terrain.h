#ifndef IL_COMMON_TERRAIN_H
#define IL_COMMON_TERRAIN_H

#include "common/matrix.h"

typedef struct il_terrain il_terrain;

il_terrain * il_terrain_new();

void il_terrain_getSize(il_terrain*, int *width, int *height);
double il_terrain_getPoint(il_terrain*, unsigned x, unsigned y, double height);
il_Vector3 il_terrain_getNormal(il_terrain*, unsigned x, unsigned y, double z);

int il_terrain_heightmapFromMemory(il_terrain*,
  int width, int height, const float * points);
int il_terrain_heightmapFromSeed(il_terrain*, long long seed,
  float resolution, float viewdistance);

#endif
