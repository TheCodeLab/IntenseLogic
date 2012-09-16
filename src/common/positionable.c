#include "positionable.h"

#include <stdlib.h>

il_Common_Positionable * il_Common_Positionable_new(il_Common_World * parent) {
  if (!parent) return NULL;
  il_Common_Positionable * p = calloc(sizeof(il_Common_Positionable),1);
  p->parent = parent;
  parent->refs++;
  p->refs = 1;
  return p;
}
