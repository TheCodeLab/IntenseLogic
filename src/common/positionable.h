#ifndef IL_COMMON_POSITIONABLE_H
#define IL_COMMON_POSITIONABLE_H

#include <sys/time.h>

#include "common/quaternion.h"
#include "common/vector.h"
#include "common/world.h"

struct ilG_drawable3d;
struct ilG_material;
struct ilG_texture;

typedef struct il_positionable {
  il_Vector3 position;
  il_Quaternion rotation;
  il_Vector3 size;
  il_Vector3 velocity;
  struct il_world *parent;
  unsigned refs;
  struct timeval last_update;
  struct ilG_drawable3d* drawable;
  struct ilG_material* material;
  struct ilG_texture* texture;
} il_positionable;

il_positionable * il_positionable_new(
  struct il_world * parent);

void il_positionable_translate(il_positionable*, il_Vector3 vec);

#endif
