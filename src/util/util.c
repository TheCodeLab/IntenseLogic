#include "loader.h"

void il_logger_init();

int il_load_ilutil() 
{
    il_load_module("ilcommon", NULL);
    il_logger_init();
    return 0;
}

