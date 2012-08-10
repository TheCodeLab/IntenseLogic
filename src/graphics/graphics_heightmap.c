#include "heightmap.h"
#include "drawable3d.h"
#include "camera.h"

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void drawquad(il_Common_Heightmap_Quad* quad, sg_Vector3 position, float s, il_Graphics_Camera* camera);

void drawMap(il_Graphics_Camera* camera, il_Graphics_Drawable3d* map) {
  il_Graphics_Heightmap* map2 = (il_Graphics_Heightmap*)map;
	/*if (!map2->vbo) {
    il_Graphics_Heightmap_redraw(map2);
  }
  glUseProgram(map2->drawable.shader);
  glBindTexture(map2->drawable.texture_target, map2->drawable.texture);
  
  glDrawArrays(GL_TRIANGLE_FAN, 0, map2->num_vertices);*/
  
  drawquad(map2->heightmap->root, (sg_Vector3){-50, 0, -50}, 100, camera);
  //drawquad(map2->heightmap->root, map2->drawable.positionable->position.x, map2->drawable.positionable->position.y, 100);
}

il_Graphics_Heightmap* il_Graphics_Heightmap_new(il_Common_Heightmap *heightmap) {
	il_Graphics_Heightmap* map = malloc(sizeof(il_Graphics_Heightmap));
	map->heightmap = heightmap;
	map->drawable.draw = &drawMap;
	return map;
}

void il_Graphics_Heightmap_redraw(il_Graphics_Heightmap *heightmap) {

}

void drawquad(il_Common_Heightmap_Quad* quad, sg_Vector3 position, float s, il_Graphics_Camera* camera) {
	float size = s * pow(2, -(float)quad->depth);
	float x = position.x;
	float y = position.y;
	float z = position.z;
	float distance = pow(x + size / 2 - camera->positionable->position.x, 2) + pow((quad->heights[0] + quad->heights[1] + quad->heights[2] + quad->heights[3]) / 4, 2) + pow(z + size / 2 - camera->positionable->position.z, 2);
	if (quad->packed.numChildren != 0 && distance <	pow((10 - quad->depth) * s / (quad->depth + 1) / 2, 2)) {
		drawquad(quad->children[0], position, s, camera);
		drawquad(quad->children[1], sg_Vector3_add(position, (sg_Vector3){size / 2, 0, 0}), s, camera);
		drawquad(quad->children[2], sg_Vector3_add(position, (sg_Vector3){size / 2, 0, size / 2}), s, camera);
		drawquad(quad->children[3], sg_Vector3_add(position, (sg_Vector3){0, 0, size / 2}), s, camera);
	} else {
		glBegin(GL_TRIANGLE_FAN);
		glNormal3f(quad->normals[0].x, quad->normals[0].y, quad->normals[0].z);
		glVertex3f(x + size / 2, (quad->heights[0] + quad->heights[1] + quad->heights[2] + quad->heights[3]) / 4, z + size / 2);
		glVertex3f(x, quad->heights[0], z);
		glVertex3f(x + size, quad->heights[1], z);

		glNormal3f(quad->normals[1].x, quad->normals[1].y, quad->normals[1].z);
		glVertex3f(x + size, quad->heights[2], z + size);

		glNormal3f(quad->normals[2].x, quad->normals[2].y, quad->normals[2].z);
		glVertex3f(x, quad->heights[3], z + size);

		glNormal3f(quad->normals[3].x, quad->normals[3].y, quad->normals[3].z);
		glVertex3f(x, quad->heights[0], z);
		glEnd();
	}
}
