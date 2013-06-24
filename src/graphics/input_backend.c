#include "input/input.h"

#include "graphics/context.h"
#include "util/log.h"

#include <GLFW/glfw3.h>

static int win_getkey(ilI_backend *self, enum ilI_key key)
{
    ilG_context *ctx = self->user;
    if (key < 512) { // the current keys map directly to glfw, so we don't need a huge table
        return glfwGetKey(ctx->window, key);
    }
    if (key < 768) {
        return glfwGetMouseButton(ctx->window, key - 512);
    }
    return 0;
}

static int joy_getkey(ilI_backend *self, enum ilI_key key)
{
    int joy = *(int*)self->user;
    int count;
    const unsigned char *buttons = glfwGetJoystickButtons(joy, &count);
    if (key - 768 >= (unsigned)count) {
        return 0;
    }
    return buttons[key - 768];
}

void ilG_registerJoystickBackend()
{
    int joy;
    for (joy = 0; glfwJoystickPresent(joy); joy++) {
        ilI_backend *backend = calloc(1, sizeof(ilI_backend));
        backend->name = strdup(glfwGetJoystickName(joy));
        il_log("Detected joystick %s", backend->name);
        backend->get = joy_getkey;
        int *user = calloc(1, sizeof(int));
        *user = joy;
        backend->user = user;
        ilI_register(backend);
    }
}

void ilG_registerInputBackend(ilG_context *ctx)
{
    ilI_backend *backend = calloc(1, sizeof(ilI_backend));
    backend->name = "GLFW Window";
    backend->get = win_getkey;
    backend->user = ctx;
    ilI_register(backend);
}

