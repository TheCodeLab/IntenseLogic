#ifndef ILG_TRANSPARENCYPASS_H
#define ILG_TRANSPARENCYPASS_H

#include "graphics/stage.h"

struct ilG_context;
struct ilG_renderer;

typedef struct ilG_transparency ilG_transparency;

extern const ilG_stagable ilG_transparency_stage;

ilG_transparency *ilG_transparency_new(struct ilG_context *context);

#endif

