#include "ilmath.h"

const il_allocator *il_math_allocator;

void il_math_setAllocator(il_allocator *alloc)
{
    il_math_allocator = alloc;
}

void *il_math_alloc(size_t size)
{
    if (!il_math_allocator) {
        il_math_allocator = il_allocator_aligned(NULL, 16);
    }
    return il_alloc(il_math_allocator, size);
}

void il_math_free(void *ptr)
{
    if (!il_math_allocator) {
        il_math_allocator = il_allocator_aligned(NULL, 16);
    }
    il_free(il_math_allocator, ptr);
}

int il_bootstrap(int argc, char **argv)
{
    return 1;
}

