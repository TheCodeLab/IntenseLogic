#include "positionable.h"

#include <stdlib.h>

#include "common/log.h"

il_positionable * il_positionable_new()
{
    //if (!parent) return NULL;
    il_positionable * p = calloc(1,sizeof(il_positionable));
    //p->parent = parent;
    p->position = il_vec4_new();
    p->size = il_vec4_set(NULL, 1, 1, 1, 1);
    p->rotation = il_quat_set(NULL, 0, 0, 0, 1);
    p->velocity = il_vec4_new();
    //parent->gc.refs++;
    p->gc.refs = 1;
    p->gc.copy = il_GC_shallowcopy;
    p->gc.free = il_GC_shallowfree;
    //il_world_add(parent, p);
    return p;
}

void il_positionable_translate(il_positionable* pos, float x, float y, float z)
{
    il_vec4 res = il_vec4_set(NULL, x, y, z, 1);
    res = il_vec4_rotate(res, pos->rotation, res);
    il_log(5, "%f %f %f -> %f %f %f", x, y, z, res[0], res[1], res[2]);
    pos->position = il_vec4_add(pos->position, res, pos->position);
    il_vec4_free(res);
}
