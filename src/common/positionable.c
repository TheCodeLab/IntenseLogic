#include "positionable.h"

#include <stdlib.h>

static il_base *positionable_new(il_type *t)
{
    (void)t;
    il_positionable * p = calloc(1,sizeof(il_positionable));
    p->base.refs = 1;
    p->base.size = sizeof(il_positionable);
    p->base.type = &il_positionable_type;
    p->position = il_vec3_new();
    p->size = il_vec3_set(NULL, 1, 1, 1);
    p->rotation = il_quat_set(NULL, 0, 0, 0, 1);
    p->velocity = il_vec3_new();
    return &p->base;
}

il_type il_positionable_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .create = positionable_new,
    .name = "il.positionable"
};

il_positionable * il_positionable_new()
{
    return (il_positionable*)il_positionable_type.create(&il_positionable_type);
}

void il_positionable_translate(il_positionable* pos, float x, float y, float z)
{
    il_vec3 res = il_vec3_set(NULL, x, y, z);
    res = il_vec3_rotate(res, pos->rotation, res);
    pos->position = il_vec3_add(pos->position, res, pos->position);
    il_vec3_free(res);
}
