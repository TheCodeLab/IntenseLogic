#include "world.h"

#include <stdlib.h>

#include <uthash.h>

#include "common/world.h"

struct hash_entry {
    ilG_world * world;
    UT_hash_handle hh;
} *hash_table = NULL;

ilG_world * ilG_world_new()
{
    return ilG_world_new_world(il_world_new());
}

ilG_world * ilG_world_new_world(il_world * world)
{
    ilG_world * w = calloc(1, sizeof(ilG_world));
    w->world = world;
    world->refs++;
    w->refs = 1;
    struct hash_entry * ent = calloc(1, sizeof(struct hash_entry));
    ent->world = w;
    HASH_ADD_PTR(hash_table, world, ent);
    return w;
}

ilG_world * il_world_getGraphicsWorld(il_world * world)
{
    struct hash_entry * out = NULL;
    HASH_FIND_PTR(hash_table, world, out);
    if (!out) return NULL;
    return out->world;
}
