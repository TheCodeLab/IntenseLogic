#ifndef ILG_HEIGHTMAP_H
#define ILG_HEIGHTMAP_H

#include "graphics/renderer.h"

struct ilG_context;
struct ilG_tex;

ilG_builder ilG_heightmap_builder(unsigned w, unsigned h, struct ilG_tex height, struct ilG_tex normal, struct ilG_tex color);

#endif

