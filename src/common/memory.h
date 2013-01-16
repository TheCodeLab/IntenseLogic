#ifndef IL_MEMORY_H
#define IL_MEMORY_H

#include <stddef.h>

struct il_GC;

typedef struct il_GC* (*il_GC_copy)(struct il_GC*);
typedef void (*il_GC_free)(struct il_GC*);

typedef struct il_GC {
    il_GC_copy copy;
    il_GC_free free;
    unsigned refs;
    struct il_GC *next;
    size_t size;
    ptrdiff_t baseptr;
} il_GC;

il_GC* global_heap;

// Assumes the GC object is inside of the memory region it represents
il_GC* il_GC_shallowcopy(il_GC*);

void il_GC_shallowfree(il_GC*);

void il_GC_add(il_GC* heap, il_GC* obj);

void il_GC_collect(il_GC* heap);

#endif

