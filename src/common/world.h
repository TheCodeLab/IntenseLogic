/** @file common/world.h
 * @brief Container for data related to in-game scenes
 */

#ifndef IL_COMMON_WORLD_H
#define IL_COMMON_WORLD_H

#include <stdlib.h>

#include "util/array.h"
#include "common/storage.h"
#include "math/vector.h"
#include "math/quaternion.h"

typedef struct il_positionable {
    struct il_world *world;
    size_t id;
} il_positionable;

typedef struct il_world {
    il_table storage;
    IL_ARRAY(il_vec3,) positions;
    IL_ARRAY(il_quat,) rotations;
    IL_ARRAY(il_vec3,) sizes;
    IL_ARRAY(il_vec3,) velocities;
    IL_ARRAY(struct timeval,) last_updates;
    IL_ARRAY(il_table,) storages;
    IL_ARRAY(size_t,) free_list;
} il_world;

il_world* il_world_new(size_t prealloc);
void il_world_free(il_world *self);

size_t il_world_numPositionables(const il_world *self);
il_positionable il_world_iter(il_world *self);
il_positionable il_world_next(const il_positionable *self);
const il_table *il_world_getStorage(const il_world *self);
il_table *il_world_mgetStorage(il_world *self);

il_positionable il_positionable_new(il_world *self);
void il_positionable_destroy(il_positionable self);

il_world *il_positionable_getWorld(const il_positionable *self);
il_vec3 il_positionable_getPosition(const il_positionable *self);
il_quat il_positionable_getRotation(const il_positionable *self);
il_vec3 il_positionable_getSize(const il_positionable *self);
il_vec3 il_positionable_getVelocity(const il_positionable *self);
struct timeval il_positionable_getLastUpdate(const il_positionable *self);
const il_table *il_positionable_getStorage(const il_positionable *self);
il_table *il_positionable_mgetStorage(il_positionable *self);

void il_positionable_setPosition(il_positionable *self, il_vec3 pos);
void il_positionable_setRotation(il_positionable *self, il_quat rot);
void il_positionable_setSize(il_positionable *self, il_vec3 size);
void il_positionable_setVelocity(il_positionable *self, il_vec3 vel);
void il_positionable_setLastUpdate(il_positionable *self, struct timeval tv);

#endif

