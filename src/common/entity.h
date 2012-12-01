#ifndef IL_COMMON_ENTITY_H
#define IL_COMMON_ENTITY_H

#include "matrix.h"
#include "positionable.h"

typedef struct il_entFocus {
  enum {IL_COMMON_FOCUSPOSITION, IL_COMMON_FOCUSDIR, IL_COMMON_FOCUSENTITY} type;
  union {sg_Vector3* vector; sg_Matrix* matrix; il_positionable* positionable;} target;
} il_entFocus;

struct il_entity;

typedef sg_Vector3 (*il_entityMoveFunc) (
  void **movedata,
  const struct il_entity *ent,
  sg_Vector3 position,
  sg_Vector3 target );

typedef sg_Vector3 (*il_entityPathfindFunc) (
  void **pathdata,
  const struct il_entity *ent,
  sg_Vector3 position,
  il_entFocus target );

typedef struct il_entity {
  il_positionable *positionable;
  int canmove;
  int living;
  il_entFocus look;
  il_entFocus move;
  float movestride;
  sg_Vector3 pathtarget;
  unsigned refs;
  float walkspeed;
  void *move_data;
  il_entityMoveFunc movefunc;
  void *path_data;
  il_entityPathfindFunc pathfinder;
} il_entity;

il_entity* il_entity_new(il_positionable *parent);

#endif
