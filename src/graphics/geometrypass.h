#ifndef ILG_GEOMETRYPASS_H
#define ILG_GEOMETRYPASS_H

#include "graphics/stage.h"

typedef struct ilG_geometry ilG_geometry;

struct ilG_context;
struct ilG_renderer;

extern const ilG_stagable ilG_geometry_stage;

struct ilG_geometry *ilG_geometry_new(struct ilG_context *context);

#endif

