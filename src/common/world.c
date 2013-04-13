#include "world.h"

#include <stdlib.h>
#include <string.h>

#include "common/positionable.h"

struct il_worldIterator {
    size_t n;
};

static il_base *world_new(il_type* t)
{
    (void)t;
    il_world * w = calloc(1, sizeof(il_world));
    w->base.refs = 1;
    w->base.type = &il_world_type;
    w->base.size = sizeof(il_world);
    return &w->base;
}

il_type il_world_type = {
    .typeclasses = NULL,
    .metadata = NULL,
    .create = world_new,
    .name = "il.world"
};

il_world* il_world_new()
{
    return (il_world*)il_world_type.create(&il_world_type);
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
