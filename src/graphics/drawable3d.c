#include "drawable3d.h"

#include "graphics/tracker.h"

static void drawable3d_init(void* self)
{
    ilG_drawable3d *drawable = self;
    ilG_drawable3d_assignId(drawable);
}

il_type ilG_drawable3d_type = {
    .typeclasses = NULL,
    .storage = {NULL},
    .constructor = &drawable3d_init,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.drawable3d",
    .size = sizeof(ilG_drawable3d),
    .parent = NULL
};

