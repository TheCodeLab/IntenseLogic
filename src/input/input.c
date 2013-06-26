#include "input.h"

#include "util/array.h"

int il_bootstrap(int argc, char **argv)
{
    (void)argc, (void)argv;
    return 0;
}

IL_ARRAY(ilI_backend*,) backends;

int ilI_getKey(enum ilI_key key, int *input)
{
    unsigned i;
    for (i = 0; i < backends.length; i++) {
        if (backends.data[i]->get(backends.data[i], key)) {
            if (input) {
                *input = i;
            }
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

char *ilI_backend_getName(int input)
{
    return strdup(backends.data[input]->name);
}

