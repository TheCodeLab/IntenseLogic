#include "drawable3d.h"

il_type ilG_drawable3d_type = {
    .typeclasses = NULL,
    .storage = {NULL},
    .constructor = NULL,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.drawable3d",
    .size = sizeof(ilG_drawable3d),
    .parent = NULL
};

