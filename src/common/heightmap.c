#include "heightmap.h"

#include <stdlib.h>

struct il_heightmap {
    unsigned int width, height;
    float *data;
};

il_heightmap* il_heightmap_new()
{
    il_heightmap* h = calloc(1, sizeof(il_heightmap));
    return h;
}

void il_heightmap_data(il_heightmap* self, unsigned int width, unsigned int height, const float *data)
{
    self->width = width;
    self->height = height;
    float highest = 0.0f;
    unsigned x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            float point = data[y * width + x];
            if (point > highest) {
                highest = point;
            }
        }
    }
    float *buf = calloc(width * height, sizeof(float));
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            buf[y * width + x] = data[y * width + x] / highest; // normalise the heights so it fits into its region
        }
    }
    self->data = buf;
}

void il_heightmap_generate(il_heightmap* self, unsigned int width, unsigned int height, unsigned int x, unsigned int y, const char *generator, unsigned long long seed)
{
    (void)self, (void)width, (void)height, (void)x, (void)y, (void)generator, (void)seed;
    // TODO: terrain generator
}

float* il_heightmap_get(const il_heightmap* self, unsigned int *width, unsigned int *height)
{
    if (width) {
        *width = self->width;
    }
    if (height) {
        *height = self->height;
    }
    return self->data;
}

