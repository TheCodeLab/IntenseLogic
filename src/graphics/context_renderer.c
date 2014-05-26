#include "graphics/context.h"

#include "util/log.h"
#include "util/logger.h"
#include "input/input.h"
#include "graphics/glutil.h"

void queue_free(struct ilG_context_queue *queue);

static void context_free(void *ptr)
{
    (void)ptr;
}

static void context_draw(void *ptr, ilG_rendid id)
{
    (void)ptr, (void)id;
    ilG_testError("Unknown");
}

bool ilG_context_build(void *ptr, ilG_rendid id, ilG_context *self, ilG_buildresult *out)
{
    (void)ptr;
    ilG_context_addName(self, id, "Context");
    *out = (ilG_buildresult) {
        .free = context_free,
        .update = context_draw,
        .draw = NULL,
        .types = NULL,
        .num_types = 0,
        .obj = self
    };
    return true;
}
