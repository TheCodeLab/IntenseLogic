#include "heightmap.h"
#include "drawable3d.h"
#include "GL/gl.h"

#include <stdlib.h>
#include <stdio.h>

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

il_Graphics_Heightmap* il_Graphics_Heightmap_new(float* heights) {
	il_Graphics_Heightmap* map = malloc(sizeof(il_Graphics_Heightmap));
	il_Graphics_Heightmap_Quad* root = il_Graphics_Heightmap_Quad_new(heights, 0);
	map->drawable = malloc(sizeof(il_Graphics_Drawable3d));
	map->root = root;
	map->drawable->draw = &drawMap;
	map->size = 8;
	return map;
}

void il_Graphics_Heightmap_Quad_draw(il_Graphics_Heightmap_Quad* quad, float size, float x, float y) {
	if (quad->numChildren != 0) {
                il_Graphics_Heightmap_Quad_draw(quad->children[0], size / 2, x, y);
		il_Graphics_Heightmap_Quad_draw(quad->children[1], size / 2, x + size / 2, y);
                il_Graphics_Heightmap_Quad_draw(quad->children[2], size / 2, x + size / 2, y + size / 2);
                il_Graphics_Heightmap_Quad_draw(quad->children[3], size / 2, x, y + size / 2);
	} else {
		glBegin(GL_QUADS);
	sg_Vector3 normal = sg_Vector3_normalise(sg_Vector3_cross((sg_Vector3){size, quad->heights[1] - quad->heights[0], 0}, (sg_Vector3){0, quad->heights[2] - quad->heights[1], size}));
		glNormal3f(normal.x, normal.y, normal.z);
		
		glVertex3f(x, quad->heights[0], y);
		glVertex3f(x + size, quad->heights[1], y);
		glVertex3f(x + size, quad->heights[2], y + size);
		glVertex3f(x, quad->heights[3], y + size);
		glEnd();
	}
}

void drawMap(il_Graphics_Drawable3d* map) {
        il_Graphics_Heightmap* map2 = (il_Graphics_Heightmap*)map;
	il_Graphics_Heightmap_Quad_draw(map2->root, map2->size, -10, -10);
}

void il_Graphics_Heightmap_Quad_divide(il_Graphics_Heightmap_Quad* parent, float scale) {
	if (parent->numChildren == 0) {	
		float error = ((float)rand() / RAND_MAX) * scale - scale / 2;
		float midHeights[] = {parent->heights[0] + (parent->heights[1] - parent->heights[0]) / 2, 
							  parent->heights[1] + (parent->heights[2] - parent->heights[1]) / 2, 
							  parent->heights[2] + (parent->heights[3] - parent->heights[2]) / 2, 
							  parent->heights[3] + (parent->heights[0] - parent->heights[3]) / 2, 
							  parent->heights[0] + (parent->heights[2] - parent->heights[0]) / 2 + error};
	
		int i;
		for (i = 0; i < 4; i++) {
			float newHeights[] = {0, 0, 0, 0}; 
			newHeights[i] = parent->heights[i];
			newHeights[(i + 1) % 4] = midHeights[i];
			newHeights[(i + 2) % 4] = midHeights[4];
			newHeights[(i + 3) % 4] = midHeights[(i + 3) % 4];
			parent->children[i] = il_Graphics_Heightmap_Quad_new(newHeights, parent->depth + 1);
		}
	
		parent->numChildren = 4;
	} else {
		int i;	
		for (i = 0; i < 4; i++) {
			il_Graphics_Heightmap_Quad_divide(parent->children[i], scale * 3. / 6);
		}
	}
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
