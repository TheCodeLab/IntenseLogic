#include "input.h"

#include "util/array.h"

int il_bootstrap(int argc, char **argv)
{
    (void)argc, (void)argv;
    return 0;
}

IL_ARRAY(ilI_backend*,) backends;

int ilI_getKey(enum ilI_key key)
{
    unsigned i;
    for (i = 0; i < backends.length; i++) {
        if (backends.data[i]->get(backends.data[i], key)) {
            return 1;
        }
    }
    return 0;
}

int ilI_register(ilI_backend *backend)
{
    IL_APPEND(backends, backend);
    return backends.length-1;
}

