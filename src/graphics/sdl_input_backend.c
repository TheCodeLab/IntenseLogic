#include <SDL2/SDL_events.h>

#include "util/event.h"
#include "graphics/context.h"

static int getkey(ilI_backend *self, enum ilI_key key)
{
    (void)self;
    int len;
    const Uint8 *keys = SDL_GetKeyboardState(&len);
    if ((int)key < len) {
        return keys[key];
    }
    if (key < 768) {
        return 0 != (SDL_GetMouseState(NULL,NULL) & 1<<(key - 512));
    }

    return 0;
}

static void handle_windowevent(ilG_context *context, SDL_Event *ev)
{
    switch (ev->window.event) {
    case SDL_WINDOWEVENT_RESIZED:
        ilG_context_resize(context, ev->window.data1, ev->window.data2);
        break;
    case SDL_WINDOWEVENT_CLOSE:
        {
            il_value nil = il_value_nil();
            ilE_handler_fire(&context->close, &nil);
            il_value_free(nil);
            break;
        }
    }
}

static void handle_keyboardevent(ilG_context *context, SDL_Event *ev)
{
    il_value v = il_value_vectorl(6,
        il_value_int(ev->key.keysym.sym),
        il_value_bool(ev->type == SDL_KEYDOWN),
        il_value_bool(ev->key.repeat),
        il_value_int(ev->key.keysym.scancode),
        il_value_int(ev->key.keysym.mod),
        il_vopaque(context, NULL)
    );
    ilE_handler_fire(context->handler.button, &v);
    il_value_free(v);
}

static void handle_mousemoveevent(ilG_context *context, SDL_Event *ev)
{
#define i il_value_int
    il_value v = il_value_vectorl(7,
        i(ev->motion.x),    i(ev->motion.y),
        i(ev->motion.xrel), i(ev->motion.yrel),
        il_value_bool(ev->motion.which == SDL_TOUCH_MOUSEID),
        il_value_int(ev->motion.state == SDL_PRESSED),
        il_vopaque(context, NULL)
    );
#undef i
    ilE_handler_fire(context->handler.mousemove, &v);
    il_value_free(v);
}

static void handle_mousebutton(ilG_context *context, SDL_Event *ev)
{
    il_value v = il_value_vectorl(6,
        il_value_int(ev->button.button + 512),
        il_value_bool(ev->button.state == SDL_PRESSED),
        il_value_bool(ev->button.which == SDL_TOUCH_MOUSEID),
        il_value_int(ev->button.x),
        il_value_int(ev->button.y),
        il_vopaque(context, NULL)
    );
    ilE_handler_fire(context->handler.button, &v);
    il_value_free(v);
}

static void handle_mousewheel(ilG_context *context, SDL_Event *ev)
{
    il_value v = il_value_vectorl(3,
        il_value_int(ev->wheel.x),
        il_value_int(ev->wheel.y),
        il_vopaque(context, NULL)
    );
    ilE_handler_fire(context->handler.mousescroll, &v);
    il_value_free(v);
}

void ilG_pollSDLEvents(ilG_context *context)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_WINDOWEVENT: handle_windowevent(context, &ev); break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: handle_keyboardevent(context, &ev); break;
        case SDL_MOUSEMOTION: handle_mousemoveevent(context, &ev); break;
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN: handle_mousebutton(context, &ev); break;
        case SDL_MOUSEWHEEL: handle_mousewheel(context, &ev); break;
        }
    }
}

void ilG_registerSdlInputBackend()
{
    ilI_backend *backend = calloc(1, sizeof(ilI_backend));
    backend->name = "SDL";
    backend->get = getkey;
    backend->user = NULL;
    ilI_register(backend);
}
