#ifndef ILG_FLOATSPACE_H
#define ILG_FLOATSPACE_H

#include <pthread.h>

#include "common/world.h"
#include "graphics/renderer.h"
#include "math/matrix.h"

struct ilG_context;

typedef struct ilG_floatspace {
    il_world *world;
    il_positionable camera;
    il_mat projection;
    unsigned id;
    pthread_mutex_t mtx;
} ilG_floatspace;

ilG_floatspace *ilG_floatspace_new(il_world *world);
void ilG_floatspace_build(ilG_floatspace *self, struct ilG_context *context);
void ilG_floatspace_addPos(ilG_floatspace *self, ilG_handle r, il_positionable p);
void ilG_floatspace_delPos(ilG_floatspace *self, ilG_handle r, il_positionable p);

#endif
