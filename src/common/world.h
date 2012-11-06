#ifndef IL_COMMON_WORLD_H
#define IL_COMMON_WORLD_H

#include <stdlib.h>

#include "common/positionable.h"

struct il_Common_Positionable;

typedef struct il_Common_World {
  unsigned refs;
  size_t nobjects;
  struct il_Common_Positionable** objects;
} il_Common_World;

typedef struct il_Common_WorldIterator il_Common_WorldIterator;

il_Common_World* il_Common_World_new();

void il_Common_World_add(il_Common_World*, struct il_Common_Positionable*);

struct il_Common_Positionable * il_Common_World_iterate(il_Common_World*, 
  il_Common_WorldIterator**);

#endif
