#include "positionable.h"

#include <stdlib.h>

#include "common/log.h"

il_Common_Positionable * il_Common_Positionable_new(il_Common_World * parent) {
  if (!parent) return NULL;
  il_Common_Positionable * p = calloc(sizeof(il_Common_Positionable),1);
  p->parent = parent;
  p->size = (sg_Vector3){1,1,1};
  p->rotation = (sg_Quaternion){0,0,0,1};
  parent->refs++;
  p->refs = 1;
  il_Common_World_add(parent, p);
  return p;
}

void il_Common_Positionable_translate(il_Common_Positionable* pos, sg_Vector3 vec) {
  sg_Vector3 res = sg_Vector3_rotate_q(vec, pos->rotation);
  il_Common_log(5, "%f %f %f -> %f %f %f", vec.x, vec.y, vec.z, res.x, res.y, res.z);
  pos->position = sg_Vector3_add(pos->position, res);
}
