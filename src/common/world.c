#include "world.h"

#include <stdlib.h>
#include <string.h>

#include "common/positionable.h"

struct il_worldIterator {
    size_t n;
};

il_type il_world_type = {
    .typeclasses = NULL,
    .storage = {NULL},
    .constructor = NULL,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.common.world",
    .size = sizeof(il_world),
    .parent = NULL
};

il_world* il_world_new()
{
    return (il_world*)il_new(&il_world_type);
}

void il_world_add(il_world* self, il_positionable* pos)
{
    IL_APPEND(self->objects, pos);
}

il_positionable * il_world_iterate(il_world* self,
        il_worldIterator** iter)
{
    if (*iter == NULL)
        *iter = calloc(1, sizeof(il_worldIterator));

    if ((*iter)->n >= self->objects.length)
        return NULL;

    return self->objects.data[(*iter)->n++];
}
