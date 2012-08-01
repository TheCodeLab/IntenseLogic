#include "heightmap.h"
#include "drawable3d.h"
#include "GL/gl.h"

#include <stdlib.h>
#include <stdio.h>

il_Graphics_Heightmap_Quad* il_Graphics_Heightmap_Quad_new(float* heights, int depth, float size) {
	il_Graphics_Heightmap_Quad* quad = malloc(sizeof(il_Graphics_Heightmap_Quad));
	int i;
	for (i = 0; i < 4; i++) {
		quad->heights[i] = heights[i];
	}
	quad->depth = depth;
	quad->numChildren = 0; 
	quad->size = size;
	il_Graphics_Heightmap_Quad_calculateNormals(quad);
	return quad;
}

il_Graphics_Heightmap* il_Graphics_Heightmap_new(float* heights, float size) {
	il_Graphics_Heightmap* map = malloc(sizeof(il_Graphics_Heightmap));
	il_Graphics_Heightmap_Quad* root = il_Graphics_Heightmap_Quad_new(heights, 0, size);
	map->drawable = malloc(sizeof(il_Graphics_Drawable3d));
	map->root = root;
	map->drawable->draw = &drawMap;
	map->size = size;
	return map;
}

void il_Graphics_Heightmap_Quad_draw(il_Graphics_Heightmap_Quad* quad, float x, float y) {
	if (quad->numChildren != 0) {
                il_Graphics_Heightmap_Quad_draw(quad->children[0], x, y);
		il_Graphics_Heightmap_Quad_draw(quad->children[1], x + quad->size / 2, y);
                il_Graphics_Heightmap_Quad_draw(quad->children[2], x + quad->size / 2, y + quad->size / 2);
                il_Graphics_Heightmap_Quad_draw(quad->children[3], x, y + quad->size / 2);
	} else {
		glBegin(GL_TRIANGLE_FAN);
		glNormal3f(quad->normals[0].x, quad->normals[0].y, quad->normals[0].z);
		glVertex3f(x + quad->size / 2, (quad->heights[0] + quad->heights[1] + quad->heights[2] + quad->heights[3]) / 4, y + quad->size / 2);
		glVertex3f(x, quad->heights[0], y);
		glVertex3f(x + quad->size, quad->heights[1], y);

		glNormal3f(quad->normals[1].x, quad->normals[1].y, quad->normals[1].z);
		glVertex3f(x + quad->size, quad->heights[2], y + quad->size);

		glNormal3f(quad->normals[2].x, quad->normals[2].y, quad->normals[2].z);
		glVertex3f(x, quad->heights[3], y + quad->size);

		glNormal3f(quad->normals[3].x, quad->normals[3].y, quad->normals[3].z);
		glVertex3f(x, quad->heights[0], y);
		glEnd();
	}
}

void drawMap(il_Graphics_Drawable3d* map) {
        il_Graphics_Heightmap* map2 = (il_Graphics_Heightmap*)map;
	il_Graphics_Heightmap_Quad_draw(map2->root, -10, -10);
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
			parent->children[i] = il_Graphics_Heightmap_Quad_new(newHeights, parent->depth + 1, parent->size / 2);
		}
	
		parent->numChildren = 4;
	} else {
		int i;	
		for (i = 0; i < 4; i++) {
			il_Graphics_Heightmap_Quad_divide(parent->children[i], scale * 3. / 6);
		}
	}
}

void il_Graphics_Heightmap_Quad_calculateNormals(il_Graphics_Heightmap_Quad* quad) {
	float avHeight = (quad->heights[0] + quad->heights[1] + quad->heights[2] + quad->heights[3]) / 4;
	//quad->normals[0] = sg_Vector3_normalise(sg_Vector3_cross((sg_Vector3){quad->size, quad->heights[1] - quad->heights[0], 0}, (sg_Vector3){0, quad->heights[2] - quad->heights[1], quad->size}));

	quad->normals[0] = sg_Vector3_normalise(sg_Vector3_cross((sg_Vector3){quad->size, quad->heights[1] - quad->heights[0], 0}, (sg_Vector3){-quad->size / 2, avHeight - quad->heights[1], quad->size / 2}));

	quad->normals[1] = sg_Vector3_normalise(sg_Vector3_cross((sg_Vector3){0, quad->heights[2] - quad->heights[1], quad->size}, (sg_Vector3){-quad->size / 2, avHeight - quad->heights[2], -quad->size / 2}));
printf("%f, %f, %f\n", quad->normals[1].x, quad->normals[1].y, quad->normals[1].z);

	quad->normals[2] = sg_Vector3_normalise(sg_Vector3_cross((sg_Vector3){-quad->size, quad->heights[3] - quad->heights[2], 0}, (sg_Vector3){quad->size / 2, avHeight - quad->heights[3], -quad->size / 2}));

	quad->normals[3] = sg_Vector3_normalise(sg_Vector3_cross((sg_Vector3){0, quad->heights[0] - quad->heights[3], -quad->size}, (sg_Vector3){quad->size / 2, avHeight - quad->heights[0], quad->size / 2}));

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
