#ifndef IL_COMMON_POSITIONABLE_H
#define IL_COMMON_POSITIONABLE_H

#include <sys/time.h>

#include "common/memory.h"
#include "math/quaternion.h"
#include "math/vector.h"
#include "common/world.h"

typedef struct il_positionable {
  il_vec4 position;
  il_quat rotation;
  il_vec4 size;
  il_vec4 velocity;
  //struct il_world *parent;
  il_GC gc;
  struct timeval last_update;
  struct ilG_drawable3d* drawable;
  struct ilG_material* material;
  struct ilG_texture* texture;
} il_positionable;

il_positionable * il_positionable_new();

void il_positionable_translate(il_positionable*, float x, float y, float z);

#endif
