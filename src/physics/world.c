#include "world.h"

il_Physics_World* il_Physics_World_create(const il_Common_World* parent) {
  il_Physics_World *world = (il_Physics_World*)malloc(sizeof(il_Physics_World));
  
  *world = (il_Physics_World) {
    (il_Common_World*)parent,
    9.81,
    0,
    dWorldCreate(),
    1
  };
  
  return world;
}
