#ifndef ILG_LIGHTPASS_H
#define ILG_LIGHTPASS_H

#include "graphics/renderer.h"
#include "graphics/light.h"

struct ilG_context;

typedef struct ilG_lights ilG_lights;

extern const ilG_renderable ilG_lights_renderer;

#define ilG_lights_wrap(p) ilG_renderer_wrap(p, &ilG_lights_renderer)

ilG_lights *ilG_lights_new();
void ilG_lights_add(ilG_lights *self, ilG_light light);

#endif

