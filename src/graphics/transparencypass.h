#ifndef ILG_TRANSPARENCYPASS_H
#define ILG_TRANSPARENCYPASS_H

#include "graphics/renderer.h"

struct ilG_context;
struct ilG_renderer;

typedef struct ilG_transparency ilG_transparency;

extern const ilG_renderable ilG_transparency_renderer;

#define ilG_transparency_wrap(p) ilG_renderer_wrap(p, &ilG_transparency_renderer)

ilG_transparency *ilG_transparency_new();

#endif

