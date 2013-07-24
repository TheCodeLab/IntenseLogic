#include "common/base.h"
#include "common/event.h"

void ilE_init();

int il_bootstrap(int argc, char **argv)
{
    (void)argc, (void)argv;
    ilE_init();
    ilE_globalevent(il_registry, "startup", 0, NULL); // doesn't get handled until all modules have loaded so we're good here
    return 1;
}

