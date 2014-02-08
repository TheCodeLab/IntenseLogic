#ifndef ILG_LIGHTPASS_H
#define ILG_LIGHTPASS_H

#include "graphics/stage.h"

struct ilG_context;

typedef struct ilG_lights ilG_lights;

extern const ilG_stagable ilG_lights_stage;

ilG_lights *ilG_lights_new(struct ilG_context *context);

#endif

