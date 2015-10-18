#include "graphics.h"

ilA_fs ilG_shaders;

void ilG_shaders_addPath(const char *arg)
{
    ilA_adddir(&ilG_shaders, arg, -1);
}
