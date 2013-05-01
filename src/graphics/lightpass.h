#ifndef ILG_LIGHTPASS_H
#define ILG_LIGHTPASS_H

#include "common/base.h"

struct ilG_stage;
struct ilG_context;

struct ilG_stage *ilG_lightpass(struct ilG_context* context);

extern il_type ilG_lightpass_type;

#endif

