#include "world.h"

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>

il_world *il_world_new(size_t prealloc)
{
    il_world *self = calloc(1, sizeof(il_world));
    il_world_init(self, prealloc);
    return self;
}

void il_world_init(il_world *self, size_t prealloc)
{
    IL_RESIZE(self->positions, prealloc);
    IL_RESIZE(self->rotations, prealloc);
    IL_RESIZE(self->sizes, prealloc);
    IL_RESIZE(self->velocities, prealloc);
    IL_RESIZE(self->last_updates, prealloc);
    IL_RESIZE(self->storages, prealloc);
}

void il_world_free(il_world *self)
{
    IL_FREE(self->positions);
    IL_FREE(self->rotations);
    IL_FREE(self->sizes);
    IL_FREE(self->velocities);
    IL_FREE(self->last_updates);
    IL_FREE(self->storages);
    free(self);
}

size_t il_world_numPositionables(const il_world *self)
{
    return self->positions.length;
}

il_positionable il_world_iter(il_world *self)
{
    if (self->positions.length < 1) {
        return (struct il_positionable) {
            .world = self,
            .id = -1
        };
    }
    return (struct il_positionable) {
        .world = self,
        .id = 0
    };
}

il_positionable il_world_next(const il_positionable *self)
{
    il_positionable next = *self;
    if (++next.id >= self->world->positions.length) {
        next.id = -1;
    }
    unsigned i;
    for (i = 0; i < self->world->free_list.length; i++) {
        if (self->world->free_list.data[i] == next.id) {
            return il_world_next(&next);
        }
    }
    return next;
}

const il_table *il_world_getStorage(const il_world *self)
{
    return &self->storage;
}

il_table *il_world_mgetStorage(il_world *self)
{
    return &self->storage;
}

il_positionable il_positionable_new(il_world *self)
{
    assert(self);
    if (self->free_list.length > 0) {
        return (il_positionable) {
            .world = self,
            .id = self->free_list.data[--self->free_list.length]
        };
    }
    IL_APPEND(self->positions, il_vec3_new(0,0,0));
    IL_APPEND(self->rotations, il_quat_new(0,0,0,1));
    IL_APPEND(self->sizes, il_vec3_new(1,1,1));
    IL_APPEND(self->velocities, il_vec3_new(0,0,0));
    struct timeval tv = (struct timeval){0,0};
    IL_APPEND(self->last_updates, tv);
    IL_APPEND(self->storages, il_table_new());
    return (il_positionable) {
        .world = self,
        .id = self->positions.length-1
    };
}

void il_positionable_destroy(il_positionable self)
{
    IL_APPEND(self.world->free_list, self.id);
}

il_world *il_positionable_getWorld(const il_positionable *self)
{
    assert(self->world);
    return self->world;
}

il_vec3 il_positionable_getPosition(const il_positionable *self)
{
    assert(self->world);
    return self->world->positions.data[self->id];
}

il_quat il_positionable_getRotation(const il_positionable *self)
{
    assert(self->world);
    return self->world->rotations.data[self->id];
}

il_vec3 il_positionable_getSize(const il_positionable *self)
{
    assert(self->world);
    return self->world->sizes.data[self->id];
}

il_vec3 il_positionable_getVelocity(const il_positionable *self)
{
    assert(self->world);
    return self->world->velocities.data[self->id];
}

struct timeval il_positionable_getLastUpdate(const il_positionable *self)
{
    assert(self->world);
    return self->world->last_updates.data[self->id];
}

const il_table *il_positionable_getStorage(const il_positionable *self)
{
    assert(self->world);
    return &self->world->storages.data[self->id];
}

il_table *il_positionable_mgetStorage(il_positionable *self)
{
    assert(self->world);
    return &self->world->storages.data[self->id];
}

void il_positionable_setPosition(il_positionable *self, il_vec3 pos)
{
    assert(self->world);
    self->world->positions.data[self->id] = pos;
}

void il_positionable_setRotation(il_positionable *self, il_quat rot)
{
    assert(self->world);
    self->world->rotations.data[self->id] = rot;
}

void il_positionable_setSize(il_positionable *self, il_vec3 size)
{
    assert(self->world);
    self->world->sizes.data[self->id] = size;
}

void il_positionable_setVelocity(il_positionable *self, il_vec3 vel)
{
    assert(self->world);
    self->world->velocities.data[self->id] = vel;
}

void il_positionable_setLastUpdate(il_positionable *self, struct timeval tv)
{
    assert(self->world);
    self->world->last_updates.data[self->id] = tv;
}

