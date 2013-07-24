/** @file ilmath.h
 * @brief Functions for controlling the allocator for ilmath
 */
#ifndef IL_MATH_H
#define IL_MATH_H

#include "util/alloc.h"

/** Sets the allocator used by the library. Do not change while the library is in use - the memory currently allocated is likely unable to be freed by a new allocator */
void il_math_setAllocator(il_allocator *alloc);
/** Allocates memory using the current allocator */
void *il_math_alloc(size_t size);
/** Frees memory using the current allocator */
void il_math_free(void *ptr);

#endif

