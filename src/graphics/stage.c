#include "stage.h"

#include "stdlib.h"

il_type ilG_stage_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = NULL,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.stage",
    .size = sizeof(ilG_stage),
    .parent = NULL
};

