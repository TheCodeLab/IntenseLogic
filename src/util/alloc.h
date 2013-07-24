/** @file alloc.h
 * @brief Functions for controlling allocation
 */

#ifndef IL_ALLOC_H
#define IL_ALLOC_H

#include <stdlib.h>

typedef void *(*il_alloc_fn)(void *user, size_t size);
typedef void (*il_free_fn)(void *user, void *ptr);

typedef struct il_allocator il_allocator;

/** Wrapper around malloc(3)/free(3) */
extern const il_allocator il_default_alloc;

/** Calls the embedded il_alloc_fn() */
void *il_alloc(const il_allocator *self, size_t size);
/** Allocates memory that is cleared to zero, like calloc(3) */
void *il_calloc(const il_allocator *self, size_t nmemb, size_t size);
/** Resizes a section of memory, but also copies data if it needs to be moved (unlike realloc(3)) */
void *il_realloc(const il_allocator *self, void *ptr, size_t oldsize, size_t newsize);
/** Calls the embedded il_free_fn() */
void il_free(const il_allocator *self, void *ptr);
/** Creates a new allocator using the two functions and a user pointer */
il_allocator* il_allocator_new(const il_allocator *allocator, il_alloc_fn alloc, il_free_fn free, void *user);
/** Creates an allocator that returns data aligned to the specified alignment */
il_allocator* il_allocator_aligned(const il_allocator *allocator, size_t alignment);

#endif

