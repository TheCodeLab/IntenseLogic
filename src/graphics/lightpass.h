#ifndef ILG_LIGHTPASS_H
#define ILG_LIGHTPASS_H

#include "graphics/renderer.h"

struct ilG_context;
ilG_builder ilG_pointlight_builder(struct ilG_context *context);
ilG_builder ilG_sunlight_builder(struct ilG_context *context);

#endif
