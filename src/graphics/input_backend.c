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
    // TODO: Use struct again once native struct storage is in
    /*ilI_buttonevent ev;
    ev.button = button + 512;
    ev.scancode = 0;
    ev.device = 0;
    ev.action = action;
    ev.mods = mods;*/
    il_value val = il_value_vectorl(5, 
        il_value_int(button+512), 
        il_value_int(0), 
        il_value_int(0), 
        il_value_bool(action), 
        il_value_int(mods)
    );
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    ilE_handler_fire(ctx->handler.button, &val);
    il_value_free(val);
}

static void cursorpos(GLFWwindow *window, double x, double y)
{
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    il_vector *old = il_table_mgetsa(&ctx->storage, "input.last_mouse");
    if (!old) {
        old = il_value_tomvec(il_table_setsa(&ctx->storage, "input.last_mouse", il_vector_new(2, il_value_int(0), il_value_int(0))));
    }
    int oldx = il_vector_geti(old, 0),
        oldy = il_vector_geti(old, 1);
#define i il_value_int
    il_value val = il_value_vectorl(4, 
        i(x),       i(y),
        i(x-oldx),  i(y-oldy)
    );
#undef i
    il_vector_seti(old, 0, x);
    il_vector_seti(old, 1, y);
    ilG_context *context = glfwGetWindowUserPointer(window);
    ilE_handler_fire(context->handler.mousemove, &val);
    il_value_free(val);
}

static void cursorenter(GLFWwindow *window, int entered)
{
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    il_value val = il_value_bool(entered);
    ilE_handler_fire(ctx->handler.mouseenter, &val);
    il_value_free(val);
}
 
static void scroll(GLFWwindow *window, double x, double y)
{
    il_value arr = il_value_vectorl(2, x, y);
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    ilE_handler_fire(ctx->handler.mousescroll, &arr);
    il_value_free(arr);
}
 
static void keyfun(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    /* TODO: Native structs
    ilI_buttonevent ev;
    ev.button = key;
    ev.scancode = scancode;
    ev.device = 0;
    ev.action = action;
    ev.mods = mods;*/
    il_value ev = il_value_vectorl(5, 
        il_value_int(key), 
        il_value_int(scancode), 
        il_value_int(0), 
        il_value_bool(action), 
        il_value_int(mods)
    );
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    ilE_handler_fire(ctx->handler.button, &ev);
    il_value_free(ev);
}
 
static void charfun(GLFWwindow *window, unsigned int character)
{
    // TODO: unsigned ints
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    il_value val = il_value_int(character);
    ilE_handler_fire(ctx->handler.character, &val);
    il_value_free(val);
}

static void closewindow(GLFWwindow *window)
{
    ilG_context *ctx = glfwGetWindowUserPointer(window);
    il_value val = il_vopaque(ctx, NULL);
    ilE_handler_fireasync(ctx->close, val); // prevents closing the window while we're still rendering
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

