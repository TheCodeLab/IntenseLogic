#ifndef IL_MATH_H
#define IL_MATH_H

#include "util/alloc.h"

void il_math_setAllocator(il_allocator *alloc);
void *il_math_alloc(size_t size);
void il_math_free(void *ptr);

#endif

