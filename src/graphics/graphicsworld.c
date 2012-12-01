#include "world.h"

#include <stdlib.h>

#include <uthash.h>

#include "common/world.h"

struct hash_entry {
    il_Graphics_World * world;
    UT_hash_handle hh;
} *hash_table = NULL;

il_Graphics_World * il_Graphics_World_new()
{
    return il_Graphics_World_new_world(il_Common_World_new());
}

il_Graphics_World * il_Graphics_World_new_world(il_Common_World * world)
{
    il_Graphics_World * w = calloc(1, sizeof(il_Graphics_World));
    w->world = world;
    world->refs++;
    w->refs = 1;
    struct hash_entry * ent = calloc(1, sizeof(struct hash_entry));
    ent->world = w;
    HASH_ADD_PTR(hash_table, world, ent);
    return w;
}

il_Graphics_World * il_Common_World_getGraphicsWorld(il_Common_World * world)
{
    struct hash_entry * out = NULL;
    HASH_FIND_PTR(hash_table, world, out);
    if (!out) return NULL;
    return out->world;
}
