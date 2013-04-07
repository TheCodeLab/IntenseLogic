#ifndef IL_ALLOC_H
#define IL_ALLOC_H

#include <stdlib.h>

typedef void *(*il_alloc_fn)(void *user, size_t size);
typedef void (*il_free_fn)(void *user, void *ptr);

typedef struct il_allocator il_allocator;

extern const il_allocator il_default_alloc;

void *il_alloc(const il_allocator *self, size_t size);
void *il_calloc(const il_allocator *self, size_t nmemb, size_t size);
void *il_realloc(const il_allocator *self, void *ptr, size_t oldsize, size_t newsize);
void il_free(const il_allocator *self, void *ptr);
il_allocator* il_allocator_new(const il_allocator *allocator, il_alloc_fn alloc, il_free_fn free, void *user);
il_allocator* il_allocator_aligned(const il_allocator *allocator, size_t alignment);

#endif

