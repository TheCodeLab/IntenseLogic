#include "physics.h"

#include <ode/ode.h>

#include "common/base.h"
#include "common/event.h"

#include "world.h"

ilP_world **worlds;
unsigned numworlds;

void tick (ilE_event e)
{
    int i;
    for (i=0; i < numworlds; i++) {
        dWorldQuickStep(worlds[i]->worldid, IL_BASE_TICK);
    }
}

void ilP_init()
{
    ilE_register(IL_BASE_TICK, (ilE_callback)&tick);
}

void ilP_registerWorld(ilP_world *world)
{
    ilP_world **temp = (ilP_world**)calloc((numworlds+1), sizeof(ilP_world**));
    memcpy(temp, worlds, sizeof(ilP_world) * numworlds);
    temp[numworlds] = world;
    free(worlds);
    worlds = temp;
    numworlds++;
}
