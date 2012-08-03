#ifndef IL_COMMON_ENTITY_H
#define IL_COMMON_ENTITY_H

#include "matrix.h"
#include "positionable.h"

typedef struct il_Common_EntFocus {
  enum {IL_COMMON_FOCUSPOSITION, IL_COMMON_FOCUSDIR, IL_COMMON_FOCUSENTITY} type;
  union {sg_Vector3* vector; sg_Matrix* matrix; il_Common_Positionable* positionable;} target; 
} il_Common_EntFocus;

struct il_Common_Entity;

typedef sg_Vector3 (*il_Common_EntityMoveFunc) ( 
  void **movedata, 
  const struct il_Common_Entity *ent,
  sg_Vector3 position,
  sg_Vector3 target );

typedef sg_Vector3 (*il_Common_EntityPathfindFunc) ( 
  void **pathdata, 
  const struct il_Common_Entity *ent, 
  sg_Vector3 position, 
  il_Common_EntFocus target );

typedef struct il_Common_Entity {
  il_Common_Positionable *positionable;
  int canmove;
  int living;
  il_Common_EntFocus look;
  il_Common_EntFocus move;
  float movestride;
  sg_Vector3 pathtarget;
  unsigned refs;
  float walkspeed;
  void *move_data;
  il_Common_EntityMoveFunc movefunc;
  void *path_data;
  il_Common_EntityPathfindFunc pathfinder;
} il_Common_Entity;

il_Common_Entity* il_Common_Entity_new(il_Common_Positionable *parent);

#endif
