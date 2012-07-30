#ifndef IL_GRAPHICS_HEIGHTMAP_H
#define IL_GRAPHICS_HEIGHTMAP_H

#include "drawable3d.h"

typedef struct il_Graphics_Heightmap_Quad {
	float heights[4];
	struct il_Graphics_Heightmap_Quad* children[4];
	int depth;
	int numChildren; 
} il_Graphics_Heightmap_Quad;

typedef struct il_Graphics_Heightmap {
	il_Graphics_Drawable3d* drawable;
	il_Graphics_Heightmap_Quad* root;
	float size;
} il_Graphics_Heightmap;

il_Graphics_Heightmap_Quad* il_Graphics_Heightmap_Quad_new(float* heights, int depth);

void drawMap(il_Graphics_Drawable3d* map);

void il_Graphics_Heightmap_Quad_divide(il_Graphics_Heightmap_Quad* parent, float scale);

void il_Graphics_Heightmap_Quad_print(il_Graphics_Heightmap_Quad* quad);

il_Graphics_Heightmap* il_Graphics_Heightmap_new(float* heights);

#endif
