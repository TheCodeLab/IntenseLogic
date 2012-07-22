#include "physics.h"

#include <ode/ode.h>

#include "common/base.h"
#include "common/event.h"

#include "world.h"

il_Physics_World **worlds;
unsigned numworlds;

void tick (il_Event_Event e) {
  int i;
  for (i=0; i < numworlds; i++) {
    dWorldQuickStep(worlds[i]->worldid, IL_BASE_TICK);
  }
}

void il_Physics_init() {
  il_Event_register(IL_BASE_TICK, (il_Event_Callback)&tick);
}

void il_Physics_registerWorld(il_Physics_World *world) {
  il_Physics_World **temp = (il_Physics_World**)malloc(sizeof(il_Physics_World**) * (numworlds+1));
  memcpy(temp, worlds, sizeof(il_Physics_World) * numworlds);
  temp[numworlds] = world;
  free(worlds);
  worlds = temp;
  numworlds++;
}
