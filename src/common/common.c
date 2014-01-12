#include "common/base.h"
#include "common/event.h"

void ilE_init();

int il_bootstrap(int argc, char **argv)
{
    (void)argc, (void)argv;
    ilE_init();
    return 0;
}

