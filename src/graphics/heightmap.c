#include "heightmap.h"

il_Graphics_Heightmap_Quad* il_Graphics_Heightmap_Quad_new(float* heights, int depth) {
	il_Graphics_Heightmap_Quad* quad = malloc(sizeof(il_Graphics_Heightmap_Quad));
	int i;
	for (i = 0; i < 4; i++) {
		quad->heights[i] = heights[i];
	}
	quad->depth = depth;
	quad->numChildren = 0; 
	return quad;
}

void il_Graphics_Heightmap_Quad_divide(il_Graphics_Heightmap_Quad* parent) {
	
	float midHeights[] = {parent->heights[0] + (parent->heights[1] - parent->heights[0]) / 2, 
						  parent->heights[1] + (parent->heights[2] - parent->heights[1]) / 2, 
						  parent->heights[2] + (parent->heights[3] - parent->heights[2]) / 2, 
						  parent->heights[3] + (parent->heights[0] - parent->heights[3]) / 2, 
						  parent->heights[0] + (parent->heights[2] - parent->heights[0]) / 2};
	
	int i;
	for (i = 0; i < 4; i++) {
		float newHeights[] = {parent->heights[i], midHeights[i], midHeights[4], midHeights[(i + 3) % 4]};
		parent->children[i] = il_Graphics_Heightmap_Quad_new(newHeights, parent->depth + 1);
	}
	
	parent->numChildren = 4;
	
}

void il_Graphics_Heightmap_Quad_print(il_Graphics_Heightmap_Quad* parent) { 
	int i;
	for (i = 0; i < parent->depth; i++) {
		printf("\t");
	}
	for (i = 0; i <4; i++) {
		printf("%f ", parent->heights[i]);
	}
	printf("\n");
	
	for (i = 0; i < parent->numChildren; i++) {
		il_Graphics_Heightmap_Quad_print(parent->children[i]);
	}
	//printf("\n");
}