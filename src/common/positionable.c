#include "positionable.h"

#include <stdlib.h>

il_positionable * il_positionable_new()
{
    //if (!parent) return NULL;
    il_positionable * p = calloc(1,sizeof(il_positionable));
    //p->parent = parent;
    p->position = il_vec3_new();
    p->size = il_vec3_set(NULL, 1, 1, 1);
    p->rotation = il_quat_set(NULL, 0, 0, 0, 1);
    p->velocity = il_vec3_new();
    //parent->gc.refs++;
    p->gc.refs = 1;
    p->gc.copy = il_GC_shallowcopy;
    p->gc.free = il_GC_shallowfree;
    //il_world_add(parent, p);
    return p;
}

void il_positionable_translate(il_positionable* pos, float x, float y, float z)
{
    il_vec3 res = il_vec3_set(NULL, x, y, z);
    res = il_vec3_rotate(res, pos->rotation, res);
    pos->position = il_vec3_add(pos->position, res, pos->position);
    il_vec3_free(res);
}
