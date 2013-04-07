#include "alloc.h"

#include <string.h>

static void *default_alloc(void *user, size_t size)
{
    (void)user;
    return calloc(1, size);
}

static void default_free(void *user, void *ptr)
{
    (void)user;
    free(ptr);
}

const struct il_allocator {
    il_alloc_fn alloc;
    il_free_fn free;
    void *user;
} il_default_alloc = {
    .alloc = default_alloc,
    .free = default_free,
};

void *il_alloc(const il_allocator *self, size_t size)
{
    if (!self) {
        self = &il_default_alloc;
    }
    return self->alloc(self->user, size);
}

void *il_calloc(const il_allocator *self, size_t nmemb, size_t size)
{
    return self->alloc(self->user, nmemb * size);
}

void *il_realloc(const il_allocator *self, void *ptr, size_t oldsize, size_t newsize)
{
    if (!self) {
        self = &il_default_alloc;
    }
    void *new = self->alloc(self->user, newsize);
    memcpy(new, ptr, oldsize);
    self->free(self->user, ptr);
    return new;
}

void il_free(const il_allocator *self, void *ptr)
{
    if (!self) {
        self = &il_default_alloc;
    }
    self->free(self->user, ptr);
}

il_allocator* il_allocator_new(const il_allocator *allocator, il_alloc_fn alloc, il_free_fn free, void *user)
{
    il_allocator *obj = allocator->alloc(allocator->user, sizeof(il_allocator));
    obj->alloc = alloc;
    obj->free = free;
    obj->user = user;
    return obj;
}

#ifndef _ISOC11_SOURCE
static void *aligned_alloc(size_t align, size_t size)
{
    char *ptr = malloc(size + align);
    char *aligned = (char*)((size_t)(ptr + align) & (~(align-1)));
    aligned[-1] = aligned - ptr;
    return aligned;
}

static void aligned_free(void *ptr)
{
    free((char*)ptr - ((char*)ptr)[-1]);
}
#else
static void aligned_free(void* ptr)
{
    free(ptr);
}
#endif

static void *alloc_aligned(void *user, size_t size)
{
    return aligned_alloc(*(int*)user, size);
}

static void free_aligned(void *user, void *ptr)
{
    (void)user;
    aligned_free(ptr);
}

il_allocator* il_allocator_aligned(const il_allocator *allocator, size_t alignment)
{
    if (!allocator) {
        allocator = &il_default_alloc;
    }
    int *user = allocator->alloc(allocator->user, sizeof(int));
    *user = alignment;
    return il_allocator_new(allocator, alloc_aligned, free_aligned, user);
}

