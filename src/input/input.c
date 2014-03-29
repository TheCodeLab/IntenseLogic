#include "input.h"

#include "util/array.h"

int il_load_ilinput()
{
    ilI_handler_init(&ilI_globalHandler);
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

static void forward(const il_value *data, il_value *user)
{
    ilE_handler *parent = il_value_tomvoid(user);
    ilE_handler_fire(parent, data);
}

void ilI_handler_init(ilI_handler *self)
{
    self->button        = ilE_handler_new_with_name("il.input.button");
    self->character     = ilE_handler_new_with_name("il.input.caracter");
    self->mousemove     = ilE_handler_new_with_name("il.input.mousemove");
    self->mouseenter    = ilE_handler_new_with_name("il.input.mouseenter");
    self->mousescroll   = ilE_handler_new_with_name("il.input.mousescroll");
    if (self != &ilI_globalHandler) {
#define fwd(from, to) ilE_register(from, ILE_AFTER, ILE_ANY, forward, il_vopaque(ilI_globalHandler.to, NULL))
        fwd(self->button,       button);
        fwd(self->character,    character);
        fwd(self->mousemove,    mousemove);
        fwd(self->mouseenter,   mouseenter);
        fwd(self->mousescroll,  mousescroll);
#undef fwd
    }
}

ilI_handler ilI_globalHandler;

