#include "world.h"

#include <stdlib.h>
#include <string.h>

#include "common/positionable.h"

struct il_worldIterator {
    size_t n;
};

static void world_init(il_base *self)
{
    il_world * w = (il_world*)self;
    (void)w;
}

il_type il_world_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = world_init,
    .name = "il.world",
    .size = sizeof(il_world)
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

    if ((*iter)->n > self->objects.length)
        return NULL;

    return self->objects.data[(*iter)->n++];
}
