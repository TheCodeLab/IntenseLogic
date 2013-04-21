#include "common.h"

void ilE_init();
void ilI_init();

int il_bootstrap(int argc, char **argv)
{
    ilE_init();
    ilI_init();
    ilE_globalevent(il_registry, "startup", 0, NULL); // doesn't get handled until all modules have loaded so we're good here
    return 1;
}

