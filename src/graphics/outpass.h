#ifndef ILG_POSTPASS_H
#define ILG_POSTPASS_H

#include "graphics/renderer.h"

struct ilG_context;

typedef struct ilG_out ilG_out;

extern const ilG_renderable ilG_out_renderer;

#define ilG_out_wrap(p) ilG_renderer_wrap(p, &ilG_out_renderer)

ilG_out *ilG_out_new();

#endif

