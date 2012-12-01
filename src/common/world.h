#ifndef IL_COMMON_WORLD_H
#define IL_COMMON_WORLD_H

#include <stdlib.h>

#include "common/positionable.h"

struct il_positionable;

typedef struct il_world {
  unsigned refs;
  size_t nobjects;
  struct il_positionable** objects;
} il_world;

typedef struct il_worldIterator il_worldIterator;

il_world* il_world_new();

void il_world_add(il_world*, struct il_positionable*);

struct il_positionable * il_world_iterate(il_world*,
  il_worldIterator**);

#endif
