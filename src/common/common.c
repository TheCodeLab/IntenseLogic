#include "common/event.h"
#include "common/input.h"
#include "graphics/graphics.h"
#include "network/network.h"
/* #include "physics/physics.h" */
#include "script/script.h"
#include "asset/asset.h"

void il_init()
{
    ilE_init();
    ilI_init();
    ilG_init();
    ilN_init();
    /* ilP_init(); */
    ilS_init();
    ilA_init();
}
