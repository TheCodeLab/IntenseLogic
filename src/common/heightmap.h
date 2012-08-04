#ifndef IL_COMMON_HEIGHTMAP_H
#define IL_COMMON_HEIGHTMAP_H

#include <stdint.h>

#include "common/string.h"
#include "common/matrix.h"

typedef struct il_Common_Heightmap_Quad {
	float heights[4];
	struct il_Common_Heightmap_Quad* children[4];
	uint8_t depth;
	struct {
    unsigned numChildren :3;
    unsigned redraw      :1;
    unsigned             :0;
  } packed;
  sg_Vector3 normals[4]; // TODO: comment out when we've switched to VBOs
} il_Common_Heightmap_Quad;

typedef struct il_Common_Heightmap {
  il_Common_Heightmap_Quad* root;
} il_Common_Heightmap;

il_Common_Heightmap_Quad* il_Common_Heightmap_Quad_new(float heights[4], int depth);
il_Common_Heightmap * il_Common_Heightmap_new(float points[4]);
void il_Common_Heightmap_Quad_divide(il_Common_Heightmap_Quad * quad, size_t numPoints, const float * points);
void il_Common_Heightmap_Quad_calculateNormals(il_Common_Heightmap_Quad* quad);
const il_Common_String il_Common_Heightmap_Quad_tostring(const il_Common_Heightmap_Quad * quad);

#endif
