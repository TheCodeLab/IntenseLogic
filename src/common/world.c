#include "world.h"

#include <stdlib.h>
#include <string.h>

#include "common/positionable.h"

struct il_worldIterator {
    size_t n;
};

static il_GC* world_copy(il_GC* gc)
{
    il_world *old = (il_world*)((char*)gc + gc->baseptr), 
             *new = calloc(sizeof(il_world), 1);
    memcpy(new, old, sizeof(il_world));
    new->gc.next = NULL;
    new->gc.refs = 1;
    new->objects.data = calloc(old->objects.length, sizeof(il_positionable*));
    new->objects.capacity = old->objects.length;
    unsigned int i;
    for (i = 0; i < old->objects.length; i++) {
        il_GC* oldp = &old->objects.data[i]->gc;
        il_GC* newp = oldp->copy(oldp);
        new->objects.data[i] = (il_positionable*)((char*)newp + newp->baseptr);
    }
    return &new->gc;
}

static void world_free(il_GC* gc)
{
    il_world* world = (il_world*)((char*)gc + gc->baseptr);
    unsigned int i;
    for (i = 0; i < world->objects.length; i++) {
        world->objects.data[i]->gc.free(&world->objects.data[i]->gc);
    }
}

il_world* il_world_new()
{
    il_world * w = calloc(1, sizeof(il_world));
    w->gc.refs = 1;
    w->gc.size = sizeof(il_world);
    w->gc.copy = &world_copy;
    w->gc.free = &world_free;
    return w;
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
