#include "loader.h"

void il_logger_init();

const char **il_preload_ilutil() // TODO: Make this a soft dependency
{
    static const char *deps[] = {
        "ilcommon",
        NULL
    };
    return deps;
}

int il_load_ilutil()
{
    il_logger_init();
    return 0;
}
