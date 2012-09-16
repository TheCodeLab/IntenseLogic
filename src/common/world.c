#include "world.h"

#include <stdlib.h>

il_Common_World* il_Common_World_new() {
  il_Common_World * w = calloc(sizeof(il_Common_World),1);
  w->refs = 1;
  return w;
}
