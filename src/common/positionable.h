#ifndef IL_COMMON_POSITIONABLE_H
#define IL_COMMON_POSITIONABLE_H

#include <sys/time.h>

#include "common/quaternion.h"
#include "common/vector.h"
#include "world.h"

typedef struct il_Common_Positionable {
  sg_Vector3 position;
  sg_Quaternion rotation;
  sg_Vector3 size;
  sg_Vector3 velocity;
  il_Common_World *parent;
  unsigned refs;
  struct timeval last_update;
} il_Common_Positionable;

il_Common_Positionable * il_Common_Positionable_new(il_Common_World * parent);

void il_Common_Positionable_translate(il_Common_Positionable*, sg_Vector3 vec);

#endif
