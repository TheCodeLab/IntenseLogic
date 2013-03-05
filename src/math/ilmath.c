#include "ilmath.h"

#ifndef _ISOC11_SOURCE
void *aligned_alloc(size_t align, size_t size)
{
    char *ptr = malloc(size + align);
    char *aligned = (char*)((size_t)(ptr + align) & (~(align-1)));
    aligned[-1] = aligned - ptr;
    return aligned;
}

void aligned_free(void *ptr)
{
    free((char*)ptr - ((char*)ptr)[-1]);
}
#else
void aligned_free(void* ptr)
{
    free(ptr);
}
#endif

static void *il_math_defaultAlloc(size_t size)
{
    return aligned_alloc(16, size);
}

static void il_math_defaultFree(void *ptr)
{
    aligned_free(ptr);
}

mowgli_allocation_policy_t *il_math_policy = NULL;

void il_math_set_policy(mowgli_allocation_policy_t *policy)
{
    il_math_policy = policy;
}

void il_math_allocator_set_policy_by_name(const char *name)
{
    il_math_set_policy(mowgli_allocation_policy_lookup(name));
}

mowgli_allocation_policy_t *il_math_get_policy()
{
    if (!il_math_policy) {
        il_math_policy = mowgli_allocation_policy_create("il_math_default", il_math_defaultAlloc, il_math_defaultFree);
    }
    return il_math_policy;
}

