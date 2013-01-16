#include "common.h"

void ilE_init();
void ilI_init();

void il_init()
{
    ilE_init();
    ilI_init();
    ilG_init();
    ilN_init();
    /* ilP_init(); */
    ilA_init();
}

