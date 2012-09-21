#include "terrain.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

enum terrain_type {
  NONE = 0,
  HEIGHTMAP,
};

struct il_Common_Terrain {
  int type;
  int width, height;
  size_t size;
  void * data;
  int refs;
  void *destruct_ctx;
  void (*destruct)(il_Common_Terrain*, void*);
  void *point_ctx;
  double (*getPoint)(il_Common_Terrain*, void*, unsigned x, unsigned y, double height);
  void *normal_ctx;
  sg_Vector3 (*getNormal)(il_Common_Terrain*, void*, unsigned x, unsigned y, double z);
};

il_Common_Terrain * il_Common_Terrain_new() {
  il_Common_Terrain * ter = calloc(sizeof(il_Common_Terrain), 1);
  ter->refs = 1;
  return ter;
}

void il_Common_Terrain_getSize(il_Common_Terrain* ter, int *width, int *height) {
  if (!ter) return;
  *width = ter->width;
  *height = ter->height;
}

double il_Common_Terrain_getPoint(il_Common_Terrain* ter, unsigned x, unsigned y, double height) {
  if (!ter) return NAN;
  if (!ter->getPoint) return NAN;
  return ter->getPoint(ter, ter->point_ctx, x, y, height);
}

sg_Vector3 il_Common_Terrain_getNormal(il_Common_Terrain* ter, unsigned x, unsigned y, double z) {
  if (!ter) return (sg_Vector3){0,0,0};
  if (!ter->getNormal) return (sg_Vector3){0,0,0};
  return ter->getNormal(ter, ter->normal_ctx, x, y, z);
}

////////////////////////////////////////////////////////////////////////////////
// Heightmaps

void destruct(il_Common_Terrain* ter, void * ctx) {
  free(ter->data);
}

double heightmap_getPoint(il_Common_Terrain* ter, void * ctx, unsigned x, unsigned y, double height) {
  if (x >= ter->width || y >= ter->height) return NAN;
  return ((float*)(ter->data))[y * ter->width + x];
}

// http://www.flipcode.com/archives/Calculating_Vertex_Normals_for_Height_Maps.shtml
sg_Vector3 heightmap_getNormal(il_Common_Terrain* ter, void * ctx, unsigned x, unsigned y, double z) {
  double h(int x, int y) {
    return ((float*)ter->data)[y * ter->width + x];
  }
  float sx = h(x<ter->width-1 ? x+1 : x, y) - h(x == 0 ? x-1 : x, y);
  if (x == 0 || x == ter->width-1)
    sx *= 2;

  float sy = h(x, y<ter->height-1 ? y+1 : y) - h(x, y == 0 ?  y-1 : y);
  if (y == 0 || y == ter->height -1)
    sy *= 2;
  
  sg_Vector3 v = (sg_Vector3){-sx, 2, sy};
  return sg_Vector3_normalise(v);
}

int il_Common_Terrain_heightmapFromMemory(il_Common_Terrain* ter, int width, int height, const float * points) {
  if (width < 1 || height < 1) return -1;
  ter->width = width;
  ter->height = height;
  if (!points) return -1;
  ter->size = width * height * sizeof(float);
  ter->data = malloc(ter->size);
  memcpy(ter->data, points, ter->size);
  ter->destruct = &destruct;
  ter->getPoint = &heightmap_getPoint;
  ter->getNormal = &heightmap_getNormal;
  ter->type = HEIGHTMAP;
  return 0;
}
