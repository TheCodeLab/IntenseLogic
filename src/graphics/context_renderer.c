#include "graphics/context.h"

#include "util/log.h"
#include "util/logger.h"
#include "input/input.h"
#include "graphics/glutil.h"

void queue_free(struct ilG_context_queue *queue);

static void context_free(void *ptr, ilG_rendid id) 
{
    (void)ptr, (void)id;
}

static void context_draw(void *ptr, ilG_rendid id)
{
    (void)ptr, (void)id;
    ilG_testError("Unknown");
}

bool ilG_context_build(void *ptr, ilG_rendid id, ilG_context *self, ilG_renderer *out)
{
    (void)ptr;
    ilG_context_addName(self, id, "Context");
    *out = (ilG_renderer) {
        .id = id,
        .free = context_free,
        .draw = context_draw,
        .obj = self
    };
    return true;
}

