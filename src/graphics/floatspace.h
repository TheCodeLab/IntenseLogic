#ifndef ILG_FLOATSPACE_H
#define ILG_FLOATSPACE_H

#include <pthread.h>

#include "graphics/renderer.h"
#include "math/matrix.h"
#include "math/vector.h"

struct ilG_context;

typedef struct ilG_floatspace ilG_floatspace;

typedef struct il_pos {
    ilG_floatspace *space;
    unsigned id;
} il_pos;

struct ilG_floatspace {
    IL_ARRAY(il_vec3,) positions;
    IL_ARRAY(il_quat,) rotations;
    IL_ARRAY(il_vec3,) sizes;
    IL_ARRAY(il_vec3,) velocities;
    IL_ARRAY(size_t,) free_list;
    il_pos camera;
    il_mat projection;
    unsigned id;
    pthread_mutex_t mtx;
    ilG_renderman *rm;
};

il_pos il_pos_new(ilG_floatspace *self);
void il_pos_destroy(il_pos self);
il_vec3 il_pos_getPosition(const il_pos *self);
il_quat il_pos_getRotation(const il_pos *self);
il_vec3 il_pos_getSize(const il_pos *self);
il_vec3 il_pos_getVelocity(const il_pos *self);
void il_pos_setPosition(il_pos *self, il_vec3 pos);
void il_pos_setRotation(il_pos *self, il_quat rot);
void il_pos_setSize(il_pos *self, il_vec3 size);
void il_pos_setVelocity(il_pos *self, il_vec3 vel);

ilG_floatspace *ilG_floatspace_new();
void ilG_floatspace_init(ilG_floatspace *self, size_t prealloc);
void ilG_floatspace_free(ilG_floatspace *self);
il_mat ilG_floatspace_viewmat(ilG_floatspace *self, int type);
void ilG_floatspace_objmats(ilG_floatspace *self, il_mat *out, const unsigned *in, int type, size_t count);

#endif
