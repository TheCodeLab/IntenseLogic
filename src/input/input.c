#include "input.h"

#include "util/array.h"

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

void ilI_handler_init(ilI_handler *self)
{
    memset(self, 0, sizeof(ilI_handler));
    self->button        = ilE_handler_new_with_name("il.input.button");
    self->character     = ilE_handler_new_with_name("il.input.caracter");
    self->mousemove     = ilE_handler_new_with_name("il.input.mousemove");
    self->mouseenter    = ilE_handler_new_with_name("il.input.mouseenter");
    self->mousescroll   = ilE_handler_new_with_name("il.input.mousescroll");
}
