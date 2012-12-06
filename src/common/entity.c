#include "entity.h"

#include <stdlib.h>
#include "log.h"

il_Vector3 move_default ( void **movedata,
                          const struct il_entity *ent,
                          il_Vector3 position,
                          il_Vector3 target )
{
    (void)movedata;
    (void)ent;
    (void)position;
    return target;
}

il_Vector3 pathfind_default ( void **pathdata,
                              const struct il_entity *ent,
                              il_Vector3 position,
                              il_entFocus target )
{
    (void)pathdata;
    (void)ent;
    (void)position;
    switch(target.type) {
    case IL_COMMON_FOCUSPOSITION:
        return *target.target.vector;
    case IL_COMMON_FOCUSDIR:
    case IL_COMMON_FOCUSENTITY:
    default:
        il_log(1, "Invalid value for EntFocus.type %u", target.type);
        return (il_Vector3) {
            0,0,0
        };
    }
}

il_entity* il_entity_new(il_positionable* parent)
{
    parent->refs++;
    il_entity *ent = calloc(1, sizeof(il_entity));
    ent->positionable = parent;
    ent->living = 1;
    ent->walkspeed = 1.0f;
    ent->movefunc = &move_default;
    ent->pathfinder = &pathfind_default;
    return ent;
}
