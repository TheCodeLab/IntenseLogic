#include <SDL_events.h>

#include "common/event.h"
#include "graphics/context.h"

static int getkey(ilI_backend *self, enum ilI_key key)
{
    (void)self;
    int len;
    const Uint8 *keys = SDL_GetKeyboardState(&len);
    if ((int)key < len) {
        return keys[len];
    }
    if (key < 768) {
        return 0 != (SDL_GetMouseState(NULL,NULL) & 1<<(key - 512));
    }
    
    return 0;
}

static void handle_windowevent(SDL_Event *ev)
{
    ilG_context *context = SDL_GetWindowData(SDL_GetWindowFromID(ev->window.windowID), "context");
    switch (ev->window.event) {
    case SDL_WINDOWEVENT_RESIZED:
        ilG_context_resize(context, ev->window.data1, ev->window.data2);
        break;
    case SDL_WINDOWEVENT_CLOSE: 
        {
            il_value nil = il_value_nil();
            ilE_handler_fire(context->close, &nil);
            il_value_free(nil);
            break;
        }
    }
}

static void handle_keyboardevent(SDL_Event *ev)
{
    ilG_context *context = SDL_GetWindowData(SDL_GetWindowFromID(ev->window.windowID), "context");
    ilI_handler *handler;
    if (context) {
        handler = &context->handler;
    } else {
        handler = &ilI_globalHandler;
    }
    il_value v = il_value_vectorl(6,
        il_value_int(ev->key.keysym.sym),
        il_value_bool(ev->type == SDL_KEYDOWN),
        il_value_bool(ev->key.repeat),
        il_value_int(ev->key.keysym.scancode),
        il_value_int(ev->key.keysym.mod),
        il_vopaque(context, NULL)
    );
    ilE_handler_fire(handler->button, &v);
    il_value_free(v);
}

static void handle_mousemoveevent(SDL_Event *ev)
{
    ilG_context *context = SDL_GetWindowData(SDL_GetWindowFromID(ev->window.windowID), "context");
    ilI_handler *handler;
    if (context) {
        handler = &context->handler;
    } else {
        handler = &ilI_globalHandler;
    }
#define i il_value_int
    il_value v = il_value_vectorl(7,
        i(ev->motion.x),    i(ev->motion.y),
        i(ev->motion.xrel), i(ev->motion.yrel),
        il_value_bool(ev->motion.which == SDL_TOUCH_MOUSEID),
        il_value_int(ev->motion.state == SDL_PRESSED),
        il_vopaque(context, NULL)
    );
#undef i
    ilE_handler_fire(handler->mousemove, &v);
    il_value_free(v);
}

static void handle_mousebutton(SDL_Event *ev)
{
    ilG_context *context = SDL_GetWindowData(SDL_GetWindowFromID(ev->window.windowID), "context");
    ilI_handler *handler;
    if (context) {
        handler = &context->handler;
    } else {
        handler = &ilI_globalHandler;
    }
    il_value v = il_value_vectorl(6,
        il_value_int(ev->button.button + 512),
        il_value_bool(ev->button.state == SDL_PRESSED),
        il_value_bool(ev->button.which == SDL_TOUCH_MOUSEID),
        il_value_int(ev->button.x),
        il_value_int(ev->button.y),
        il_vopaque(context, NULL)
    );
    ilE_handler_fire(handler->button, &v);
    il_value_free(v);
}

static void handle_mousewheel(SDL_Event *ev)
{
    ilG_context *context = SDL_GetWindowData(SDL_GetWindowFromID(ev->window.windowID), "context");
    ilI_handler *handler;
    if (context) {
        handler = &context->handler;
    } else {
        handler = &ilI_globalHandler;
    }
    il_value v = il_value_vectorl(3,
        il_value_int(ev->wheel.x),
        il_value_int(ev->wheel.y),
        il_vopaque(context, NULL)
    );
    ilE_handler_fire(handler->mousescroll, &v);
    il_value_free(v);
}

static int poll_id; // TODO: Clean this up when event callbacks are shifted to worker threads
ilE_handler *poll_timer;
static void poll_events(const il_value *data, il_value *ctx)
{
    (void)data; (void)ctx;
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_WINDOWEVENT: handle_windowevent(&ev); break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: handle_keyboardevent(&ev); break;
        case SDL_MOUSEMOTION: handle_mousemoveevent(&ev); break;
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN: handle_mousebutton(&ev); break;
        case SDL_MOUSEWHEEL: handle_mousewheel(&ev); break;
        }
    }
}

void ilG_registerSdlInputBackend()
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000000 / 250; // poll 250 times per second for now
    poll_timer = ilE_handler_timer(&tv);
    ilE_handler_name(poll_timer, "SDL Event Poll");
    poll_id = ilE_register(poll_timer, ILE_DONTCARE, ILE_ANY, poll_events, il_value_nil());

    ilI_backend *backend = calloc(1, sizeof(ilI_backend));
    backend->name = "SDL";
    backend->get = getkey;
    backend->user = NULL;
    ilI_register(backend);
}

