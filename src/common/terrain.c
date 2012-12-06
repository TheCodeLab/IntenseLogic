#include "terrain.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

enum terrain_type {
    NONE = 0,
    HEIGHTMAP,
    PHEIGHTMAP,
};

struct il_terrain {
    int type;
    int width, height;
    size_t size;
    void * data;
    int refs;
    void *destruct_ctx;
    void (*destruct)(il_terrain*, void*);
    void *point_ctx;
    double (*getPoint)(il_terrain*, void*, unsigned x, unsigned y, double height);
    void *normal_ctx;
    il_Vector3 (*getNormal)(il_terrain*, void*, unsigned x, unsigned y, double z);
};

il_terrain * il_terrain_new()
{
    il_terrain * ter = calloc(1, sizeof(il_terrain));
    ter->refs = 1;
    return ter;
}

void il_terrain_getSize(il_terrain* ter, int *width, int *height)
{
    if (!ter) return;
    *width = ter->width;
    *height = ter->height;
}

double il_terrain_getPoint(il_terrain* ter, unsigned x, unsigned y, double height)
{
    if (!ter) return NAN;
    if (!ter->getPoint) return NAN;
    return ter->getPoint(ter, ter->point_ctx, x, y, height);
}

il_Vector3 il_terrain_getNormal(il_terrain* ter, unsigned x,
                                       unsigned y, double z)
{
    if (!ter) return (il_Vector3) {
        0,0,0
    };
    if (!ter->getNormal) return (il_Vector3) {
        0,0,0
    };
    return ter->getNormal(ter, ter->normal_ctx, x, y, z);
}

////////////////////////////////////////////////////////////////////////////////
// Heightmaps

static void destruct(il_terrain* ter, void * ctx)
{
    (void)ctx;
    free(ter->data);
}

static double heightmap_getPoint(il_terrain* ter, void * ctx, unsigned x,
                                 unsigned y, double height)
{
    (void)ctx;
    (void)height;
    if ((int)x >= ter->width || (int)y >= ter->height) return NAN;
    return ((float*)(ter->data))[y * ter->width + x];
}

// http://www.flipcode.com/archives/Calculating_Vertex_Normals_for_Height_Maps.shtml
static il_Vector3 heightmap_getNormal(il_terrain* ter, void * ctx,
                                      unsigned x, unsigned y, double z)
{
    (void)ctx;
    (void)z;

#define h(x,y) (((float*)ter->data)[y * ter->width + x])

    float sx = h((int)x<ter->width-1 ? (int)x+1 : (int)x, (int)y)
               - h((int)x == 0 ? (int)x-1 : (int)x, (int)y);
    if (x == 0 || (int)x == ter->width-1)
        sx *= 2;

    float sy = h((int)x, (int)y<ter->height-1 ? y+1 : y) - h(x, y == 0 ?  y-1 : y);
    if (y == 0 || (int)y == ter->height -1)
        sy *= 2;

    il_Vector3 v = (il_Vector3) {
        -sx, 2, sy
    };
    return il_Vector3_normalise(v);
}

static double pheightmap_getPoint(il_terrain* ter, void * ctx,
                                  unsigned x, unsigned y, double height)
{
    (void)ter, (void)ctx, (void)x, (void)y, (void)height;
    return NAN; // TODO: stub function
}

static il_Vector3 pheightmap_getNormal(il_terrain* ter, void * ctx,
                                       unsigned x, unsigned y, double z)
{
    (void)ter, (void)ctx, (void)x, (void)y, (void)z;
    return (il_Vector3) {
        0,0,0
    }; // TODO: stub function
}

int il_terrain_heightmapFromMemory(il_terrain* ter, int width,
        int height, const float * points)
{
    if (width < 1 || height < 1) return -1;
    ter->width = width;
    ter->height = height;
    if (!points) return -1;
    ter->size = width * height * sizeof(float);
    ter->data = calloc(1, ter->size);
    memcpy(ter->data, points, ter->size);
    ter->destruct = &destruct;
    ter->getPoint = &heightmap_getPoint;
    ter->getNormal = &heightmap_getNormal;
    ter->type = HEIGHTMAP;
    return 0;
}

struct pheightmap {
    long long seed;
    float resolution;
    float viewdistance;
};

int il_terrain_heightmapFromSeed(il_terrain* ter, long long seed,
                                        float resolution, float viewdistance)
{
    ter->size = sizeof(struct pheightmap);
    struct pheightmap* pheightmap = calloc(1, sizeof(struct pheightmap));
    pheightmap->seed = seed;
    pheightmap->resolution = resolution;
    pheightmap->viewdistance = viewdistance;
    ter->data = pheightmap;
    ter->destruct = &destruct;
    ter->getPoint = &pheightmap_getPoint;
    ter->getNormal = &pheightmap_getNormal;
    ter->type = PHEIGHTMAP;
    return 0;
}
