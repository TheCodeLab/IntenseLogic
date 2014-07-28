#ifndef HELPER_H
#define HELPER_H

#include "graphics/context.h"

typedef struct helper_result {
    ilG_context *context;
    ilG_handle sky, geom, lights, out;
} helper_result;

typedef struct helper_config {
    bool sky, geom, lights, out;
    const char *name;
    const char *skytex[6];
} helper_config;

helper_result helper_create(helper_config *conf);

#endif
