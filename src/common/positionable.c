#include "positionable.h"

#include <stdlib.h>

static void positionable_init(void *self)
{
    il_positionable * p = self;
    p->position = il_vec3_new();
    p->size = il_vec3_set(NULL, 1, 1, 1);
    p->rotation = il_quat_set(NULL, 0, 0, 0, 1);
    p->velocity = il_vec3_new();
}

il_type il_positionable_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = positionable_init,
    .name = "il.common.positionable",
    .size = sizeof(il_positionable)
};

il_positionable * il_positionable_new()
{
    return (il_positionable*)il_new(&il_positionable_type);
}

void il_positionable_translate(il_positionable* pos, float x, float y, float z)
{
    il_vec3 res = il_vec3_set(NULL, x, y, z);
    res = il_vec3_rotate(res, pos->rotation, res);
    pos->position = il_vec3_add(pos->position, res, pos->position);
    il_vec3_free(res);
}
