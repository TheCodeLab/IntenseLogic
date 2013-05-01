#ifndef ILG_POSTPASS_H
#define ILG_POSTPASS_H

#include "common/base.h"

struct ilG_context;
struct ilG_stage;

struct ilG_stage *ilG_outpass(struct ilG_context *context);

extern il_type ilG_outpass_type;

#endif

