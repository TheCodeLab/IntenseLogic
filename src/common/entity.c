#include "entity.h"

#include <stdlib.h>
#include "log.h"

sg_Vector3 move_default ( void **movedata, 
                          const struct il_Common_Entity *ent, 
                          sg_Vector3 position, 
                          sg_Vector3 target ) {
  return target;
}

sg_Vector3 pathfind_default ( void **pathdata, 
                              const struct il_Common_Entity *ent, 
                              sg_Vector3 position, 
                              il_Common_EntFocus target ) {
  switch(target.type) {
    case IL_COMMON_FOCUSPOSITION:
      return *target.target.vector;
    case IL_COMMON_FOCUSDIR:
    case IL_COMMON_FOCUSENTITY:
    default:
      il_Common_log(1, "Invalid value for EntFocus.type %u", target.type);
      return (sg_Vector3){0,0,0};
  }
}

il_Common_Entity* il_Common_Entity_new(il_Common_Positionable* parent) {
  parent->refs++;
  il_Common_Entity *ent = calloc(1, sizeof(il_Common_Entity));
  ent->positionable = parent;
  ent->living = 1;
  ent->walkspeed = 1.0f;
  ent->movefunc = &move_default;
  ent->pathfinder = &pathfind_default;
  return ent;
}
