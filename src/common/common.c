#include "common.h"

void ilE_init();
void ilI_init();

int il_bootstrap(int argc, char **argv)
{
    ilE_init();
    ilI_init();
    return 1;
}

