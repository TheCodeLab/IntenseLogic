#include "heightmap.h"

#include <stdlib.h>
#include <stdio.h>

#include "common/log.h"

il_Common_Heightmap_Quad* il_Common_Heightmap_Quad_new(float heights[4], int depth) {
  il_Common_Heightmap_Quad* quad = malloc(sizeof(il_Common_Heightmap_Quad));
  memset(quad, 0, sizeof(il_Common_Heightmap_Quad));
  memcpy(&quad->heights[0], &heights[0], 4 * sizeof(float));
	quad->depth = depth;
	quad->packed.numChildren = 0; 
	il_Common_Heightmap_Quad_calculateNormals(quad);
	return quad;
}

il_Common_Heightmap * il_Common_Heightmap_new(float points[4]) {
	il_Common_Heightmap* map = (il_Common_Heightmap*)malloc(sizeof(il_Common_Heightmap));
	il_Common_Heightmap_Quad* root = il_Common_Heightmap_Quad_new(points, 0);
	map->root = root;
	return map;
}

void il_Common_Heightmap_Quad_divide(il_Common_Heightmap_Quad * quad, size_t numPoints, const float * points) {
  float heights[9] = {
    quad->heights[0], 
    (quad->heights[0] + quad->heights[1])/2, 
    quad->heights[1],
    
    (quad->heights[0] + quad->heights[3])/2,
    (quad->heights[0] + quad->heights[1] + quad->heights[2] + quad->heights[3])/4, 
    (quad->heights[1] + quad->heights[2])/2,
    
    quad->heights[3], 
    (quad->heights[3] + quad->heights[2])/2, 
    quad->heights[2]
  };
  switch (numPoints) {
    case 0:
      break;
    case 1:
      heights[4] = *points;
      break;
    case 5:
      heights[4] = *points;
    case 4:
      heights[0] = points[0];
      heights[2] = points[1];
      heights[6] = points[2];
      heights[8] = points[3];
      break;
    case 9:
      memcpy(&heights, points, 9 * sizeof(float));
      break;
    default:
      il_Common_log(2, "Unsupported number of heights provided (%u).", numPoints);
      break;
  }
  
  // counter-clockwise
  int lookup_table[16] = {
    0, 1, 4, 3, // UL
    1, 2, 5, 4, // UR
    4, 5, 8, 7, // LR
    3, 4, 7, 6  // LL
  };
  
  int i;
  for (i = 0; i < 4; i++) {
    float subheights[4];
    int j;
    for (j=0; j<4; j++) {
      int idx = lookup_table[i*4 + j];
      subheights[j] = heights[idx];
    }
    /*unsigned offset = (i%2) + (i/2)*3;
    int j;
    for (j = 0; j < 4; j++)
      subheights[j] = heights[offset + (j%2) + (j/2)*3];*/
    
    if (quad->children[i]) {
      il_Common_Heightmap_Quad_divide(quad->children[i], 0, NULL);
    } else {
      quad->children[i] = il_Common_Heightmap_Quad_new(subheights, quad->depth+1);
    }
  }
	
  quad->packed.numChildren = (unsigned)4;

}

void il_Common_Heightmap_Quad_calculateNormals(il_Common_Heightmap_Quad* quad) {
	float avHeight = (quad->heights[0] + quad->heights[1] + quad->heights[2] + quad->heights[3]) / 4;
	float size = pow(2, -(float)quad->depth);

	quad->normals[0] = sg_Vector3_mul_f(
    sg_Vector3_normalise(
      sg_Vector3_cross(
        (sg_Vector3){size, quad->heights[1] - quad->heights[0], 0}, 
        (sg_Vector3){-size / 2, avHeight - quad->heights[1], size / 2}
      )
    ), 
    -1
  );
  
	quad->normals[1] = sg_Vector3_mul_f(
    sg_Vector3_normalise(
      sg_Vector3_cross(
        (sg_Vector3){0.0f, quad->heights[2] - quad->heights[1], size}, 
        (sg_Vector3){-size / 2, avHeight - quad->heights[2], -size / 2}
      )
    ), 
    -1
  );
  
	quad->normals[2] = sg_Vector3_mul_f(
    sg_Vector3_normalise(
      sg_Vector3_cross(
        (sg_Vector3){-size, quad->heights[3] - quad->heights[2], 0}, 
        (sg_Vector3){size / 2, avHeight - quad->heights[3], -size / 2}
      )
    ), 
    -1
  );
  
	quad->normals[3] = sg_Vector3_mul_f(
    sg_Vector3_normalise(
      sg_Vector3_cross(
        (sg_Vector3){0, quad->heights[0] - quad->heights[3], -size}, 
        (sg_Vector3){size / 2, avHeight - quad->heights[0], size / 2}
      )
    ), 
    -1
  );
}

const il_Common_String il_Common_Heightmap_Quad_tostring(const il_Common_Heightmap_Quad * quad) {
  char *s = (char*)malloc(128);
  int len = snprintf(
    s, 
    128,
    "{points=[%f %f %f %f] children=%u depth=%u}", 
    quad->heights[0], quad->heights[1], quad->heights[2], quad->heights[3], 
    (unsigned int)quad->packed.numChildren,
    (unsigned int)quad->depth
  );
  return (il_Common_String){len,s};
}
