#include "memory.h"

#include <stdlib.h>
#include <string.h>

il_GC* il_GC_shallowcopy(il_GC* gc)
{
    char * data = malloc(gc->size);
    memcpy(data, (char*)gc + gc->baseptr, gc->size);
    return (il_GC*)(data - gc->baseptr);
}

void il_GC_shallowfree(il_GC* gc)
{
    if (gc->refs > 0) return;
    free((char*)gc + gc->baseptr);
}

void il_GC_add(il_GC* heap, il_GC* obj)
{
    obj->next = heap->next;
    heap->next = obj;
}

void il_GC_collect(il_GC* heap)
{
    il_GC *cur = heap;
    il_GC *last;

    do {
        last = cur;
        cur = cur->next;
        if (cur->refs < 1) {
            last->next = cur->next;
            cur->free(cur);
        }
    } while (cur);
}


