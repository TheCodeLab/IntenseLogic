#ifndef ILG_HEIGHTMAP_H
#define ILG_HEIGHTMAP_H

#include "common/base.h"

extern il_type ilG_heightmap_type;

struct ilG_context;

struct ilG_drawable3d *ilG_heightmap_new(struct ilG_context *context, unsigned w, unsigned h);
struct ilG_material *ilG_heightmap_shader(struct ilG_context *context);

#endif

