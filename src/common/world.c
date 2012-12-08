#include "world.h"

#include <stdlib.h>
#include <string.h>

#include "common/positionable.h"

struct il_worldIterator {
    size_t n;
};

il_world* il_world_new()
{
    il_world * w = calloc(1, sizeof(il_world));
    w->refs = 1;
    return w;
}

void il_world_add(il_world* self, il_positionable* pos)
{
    il_positionable** temp = calloc(self->nobjects, sizeof(il_positionable*));
    memcpy(temp, self->objects, sizeof(il_positionable*) * self->nobjects);
    temp[self->nobjects] = pos;
    free(self->objects);
    self->objects = temp;
    self->nobjects++;
    pos->refs++;
}

il_positionable * il_world_iterate(il_world* self,
        il_worldIterator** iter)
{
    if (*iter == NULL)
        *iter = calloc(1, sizeof(il_worldIterator));

    if ((*iter)->n > self->nobjects)
        return NULL;

    return self->objects[(*iter)->n++];
}
