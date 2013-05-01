#ifndef ILG_STAGE_H
#define ILG_STAGE_H

#include "common/base.h"

typedef struct ilG_stage ilG_stage;

typedef void (*ilG_stage_run_fn)(ilG_stage*);

struct ilG_stage {
    il_base base;
    struct ilG_context *context;
    ilG_stage_run_fn run;
    const char *name;
};

extern il_type ilG_stage_type;

#endif

