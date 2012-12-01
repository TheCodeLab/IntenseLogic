#include "positionable.h"

#include <stdlib.h>

#include "common/log.h"

il_positionable * il_positionable_new(il_world * parent)
{
    if (!parent) return NULL;
    il_positionable * p = calloc(1,sizeof(il_positionable));
    p->parent = parent;
    p->size = (sg_Vector3) {
        1,1,1
    };
    p->rotation = (sg_Quaternion) {
        0,0,0,1
    };
    parent->refs++;
    p->refs = 1;
    il_world_add(parent, p);
    return p;
}

void il_positionable_translate(il_positionable* pos, sg_Vector3 vec)
{
    sg_Vector3 res = sg_Vector3_rotate_q(vec, pos->rotation);
    il_log(5, "%f %f %f -> %f %f %f", vec.x, vec.y, vec.z, res.x, res.y, res.z);
    pos->position = sg_Vector3_add(pos->position, res);
}
