#include "input/input.h"

#include "graphics/context.h"
#include "util/log.h"
#include "common/base.h"

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

static void mousebutton(GLFWwindow *window, int button, int action, int mods)
{
    ilI_buttonevent ev;
    ev.button = button + 512;
    ev.scancode = 0;
    ev.device = 0;
    ev.action = action;
    ev.mods = mods;
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    ilE_handler_fire(ctx->input_handler.button, sizeof(ilI_buttonevent), &ev);
}

static void cursorpos(GLFWwindow *window, double x, double y)
{
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    il_vector *old = il_value_tovec(il_table_gets(&ctx->base.storage, "input.last_mouse"));
    if (!old) {
        old = il_value_tovec(il_table_sets(&ctx->base.storage, "input.last_mouse", il_value_vectorl(2, il_value_int(0), il_value_int(0))));
    }
    int arr[4] = {x,y, x-il_vector_geti(old, 0), y-il_vector_geti(old, 1)};
    il_vector_seti(old, 0, x);
    il_vector_seti(old, 1, y);
    ilG_context *context = glfwGetWindowUserPointer(window);
    ilE_handler_fire(context->input_handler.mousemove, sizeof(int) * 4, &arr);
}

static void cursorenter(GLFWwindow *window, int entered)
{
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    ilE_handler_fire(ctx->input_handler.mouseenter, sizeof(int), &entered);
}
 
static void scroll(GLFWwindow *window, double x, double y)
{
    float arr[2] = {x,y};
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    ilE_handler_fire(ctx->input_handler.mousescroll, sizeof(int) * 2, &arr);
}
 
static void keyfun(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    ilI_buttonevent ev;
    ev.button = key;
    ev.scancode = scancode;
    ev.device = 0;
    ev.action = action;
    ev.mods = mods;
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    ilE_handler_fire(ctx->input_handler.button, sizeof(ilI_buttonevent), &ev);
}
 
static void charfun(GLFWwindow *window, unsigned int character)
{
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    ilE_handler_fire(ctx->input_handler.character, sizeof(unsigned int), &character);
}

static void closewindow(GLFWwindow *window)
{
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    ilE_handler_fireasync(ctx->close, sizeof(ilG_context), ctx); // prevents closing the window while we're still rendering
}

void ilG_registerInputBackend(ilG_context *ctx)
{
    ilI_backend *backend = calloc(1, sizeof(ilI_backend));
    backend->name = "GLFW Window";
    backend->get = win_getkey;
    backend->user = ctx;
    ilI_register(backend);
    glfwSetKeyCallback          (ctx->window, keyfun);
    glfwSetCharCallback         (ctx->window, charfun);
    glfwSetMouseButtonCallback  (ctx->window, mousebutton);
    glfwSetCursorPosCallback    (ctx->window, cursorpos);
    glfwSetCursorEnterCallback  (ctx->window, cursorenter);
    glfwSetScrollCallback       (ctx->window, scroll);
    glfwSetWindowCloseCallback  (ctx->window, closewindow);
}

