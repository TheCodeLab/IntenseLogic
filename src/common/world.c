#include "world.h"

#include <stdlib.h>
#include <string.h>

struct il_Common_WorldIterator {
    size_t n;
};

il_Common_World* il_Common_World_new()
{
    il_Common_World * w = calloc(1, sizeof(il_Common_World));
    w->refs = 1;
    return w;
}

void il_Common_World_add(il_Common_World* self, il_Common_Positionable* pos)
{
    il_Common_Positionable** temp = calloc(self->nobjects, sizeof(il_Common_Positionable*));
    memcpy(temp, self->objects, sizeof(il_Common_Positionable*) * self->nobjects);
    temp[self->nobjects] = pos;
    free(self->objects);
    self->objects = temp;
    self->nobjects++;
    pos->refs++;
}

il_Common_Positionable * il_Common_World_iterate(il_Common_World* self,
        il_Common_WorldIterator** iter)
{
    if (*iter == NULL)
        *iter = calloc(1, sizeof(il_Common_WorldIterator));

    if ((*iter)->n > self->nobjects)
        return NULL;

    return self->objects[(*iter)->n++];
}
