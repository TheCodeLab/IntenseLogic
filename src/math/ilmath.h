#ifndef IL_MATH_H
#define IL_MATH_H

#include <mowgli.h>

void il_math_set_policy(mowgli_allocation_policy_t *policy);
void il_math_allocator_set_policy_by_name(const char *name);
mowgli_allocation_policy_t *il_math_get_policy();

#endif

