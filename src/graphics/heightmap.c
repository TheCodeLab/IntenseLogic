#include "heightmap.h"
#include "drawable3d.h"

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void drawquad(il_Common_Heightmap_Quad* quad, float x, float y);

void drawMap(il_Graphics_Drawable3d* map) {
  il_Graphics_Heightmap* map2 = (il_Graphics_Heightmap*)map;
	/*if (!map2->vbo) {
    il_Graphics_Heightmap_redraw(map2);
  }
  glUseProgram(map2->drawable.shader);
  glBindTexture(map2->drawable.texture_target, map2->drawable.texture);
  
  glDrawArrays(GL_TRIANGLE_FAN, 0, map2->num_vertices);*/
  
  drawquad(map2->heightmap->root, map2->drawable.positionable->position.x, map2->drawable.positionable->position.y);
}

il_Graphics_Heightmap* il_Graphics_Heightmap_new(il_Common_Heightmap *heightmap) {
	il_Graphics_Heightmap* map = malloc(sizeof(il_Graphics_Heightmap));
	map->heightmap = heightmap;
	map->drawable.draw = &drawMap;
	return map;
}

void il_Graphics_Heightmap_redraw(il_Graphics_Heightmap *heightmap) {

}

void drawquad(il_Common_Heightmap_Quad* quad, float x, float y) {
  float size = pow(2, quad->depth);
	if (quad->packed.numChildren != 0) {
		drawquad(quad->children[0], x, y);
		drawquad(quad->children[1], x + size / 2, y);
		drawquad(quad->children[2], x + size / 2, y + size / 2);
		drawquad(quad->children[3], x, y + size / 2);
	} else {
		glBegin(GL_TRIANGLE_FAN);
		glNormal3f(quad->normals[0].x, quad->normals[0].y, quad->normals[0].z);
		glVertex3f(x + size / 2, (quad->heights[0] + quad->heights[1] + quad->heights[2] + quad->heights[3]) / 4, y + size / 2);
		glVertex3f(x, quad->heights[0], y);
		glVertex3f(x + size, quad->heights[1], y);

		glNormal3f(quad->normals[1].x, quad->normals[1].y, quad->normals[1].z);
		glVertex3f(x + size, quad->heights[2], y + size);

		glNormal3f(quad->normals[2].x, quad->normals[2].y, quad->normals[2].z);
		glVertex3f(x, quad->heights[3], y + size);

		glNormal3f(quad->normals[3].x, quad->normals[3].y, quad->normals[3].z);
		glVertex3f(x, quad->heights[0], y);
		glEnd();
	}
}
