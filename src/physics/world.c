#include "world.h"

ilP_world* ilP_world_create(const il_world* parent)
{
    ilP_world *world = (ilP_world*)calloc(1, sizeof(ilP_world));

    *world = (ilP_world) {
        (il_world*)parent,
        9.81,
        0,
        dWorldCreate(),
        1
    };

    return world;
}
