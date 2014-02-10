#ifndef ILG_STAGE_H
#define ILG_STAGE_H

#include "graphics/renderer.h"

typedef struct ilG_stagable {
    void (*run)(void *obj);
    int /*success*/ (*track)(void *obj, ilG_renderer r);
    const char *name;
} ilG_stagable;

typedef struct ilG_stage {
    void *obj;
    const ilG_stagable *stagable;
} ilG_stage;

ilG_stage ilG_stage_new(void *obj, const ilG_stagable *stagable);
int ilG_stage_track(ilG_stage self, ilG_renderer renderer);
const char *ilG_stage_getName(ilG_stage self);

#endif

