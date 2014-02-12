#ifndef ILG_GEOMETRYPASS_H
#define ILG_GEOMETRYPASS_H

#include "graphics/renderer.h"

typedef struct ilG_geometry ilG_geometry;

struct ilG_context;
struct ilG_renderer;

extern const ilG_renderable ilG_geometry_renderer;

ilG_geometry *ilG_geometry_new();
#define ilG_geometry_wrap(p) ilG_renderer_wrap(p, &ilG_geometry_renderer)

#endif

