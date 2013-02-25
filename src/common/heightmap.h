#ifndef IL_HEIGHTMAP_H
#define IL_HEIGHTMAP_H

typedef struct il_heightmap il_heightmap;

il_heightmap* il_heightmap_new();
void il_heightmap_data(il_heightmap* self, unsigned int width, unsigned int height, const float *data);
void il_heightmap_generate(il_heightmap* self, unsigned int width, unsigned int height, unsigned int x, unsigned int y, const char *generator, unsigned long long seed);
float* il_heightmap_get(const il_heightmap* self, unsigned int *width, unsigned int *height);


#endif

