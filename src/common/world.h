#ifndef IL_COMMON_WORLD_H
#define IL_COMMON_WORLD_H

typedef struct il_Common_World {
  unsigned refs;
} il_Common_World;

il_Common_World* il_Common_World_new();

#endif
