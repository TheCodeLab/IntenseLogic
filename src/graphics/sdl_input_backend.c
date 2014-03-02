#include <SDL_events.h>

#include "common/event.h"
#include "graphics/context.h"

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

static int poll_refs, poll_id; // TODO: Clean this up when event callbacks are shifted to worker threads
ilE_handler *poll_timer;
static void poll_events(const il_value *data, il_value *ctx)
{
    (void)data; (void)ctx;
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_WINDOWEVENT: handle_windowevent(&ev); break;
        }
    }
}

void ilG_poll_ref()
{
    if (poll_refs < 1) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 1000000 / 250; // poll 250 times per second for now
        poll_timer = ilE_handler_timer(&tv);
        poll_id = ilE_register(poll_timer, ILE_DONTCARE, ILE_ANY, poll_events, il_value_nil());
    }
    poll_refs++;
}

void ilG_poll_unref()
{
    poll_refs--;
    if (poll_refs < 1) {
        ilE_unregister(poll_timer, poll_id);
        ilE_handler_destroy(poll_timer);
    }
}

