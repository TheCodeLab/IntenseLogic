#ifndef ILG_POSTPASS_H
#define ILG_POSTPASS_H

#include "graphics/stage.h"

struct ilG_context;

typedef struct ilG_out ilG_out;

extern const ilG_stagable ilG_out_stage;

ilG_out *ilG_out_new(struct ilG_context *context);

#endif

