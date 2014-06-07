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

bool ilG_context_build(void *ptr, ilG_rendid id, ilG_context *self, ilG_buildresult *out)
{
    (void)ptr;
    ilG_context_addName(self, id, "Context");
    *out = (ilG_buildresult) {
        .free = context_free,
        .update = NULL,
        .draw = NULL,
        .view = NULL,
        .types = NULL,
        .num_types = 0,
        .obj = self
    };
    return true;
}

void ilG_default_update(void *obj, ilG_rendid id)
{
    (void)obj, (void)id;
}

void ilG_default_multiupdate(void *obj, ilG_rendid id, il_mat *mats)
{
    (void)obj, (void)id, (void)mats;
}

void ilG_default_draw(void *obj, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats)
{
    (void)obj, (void)id, (void)mats, (void)objects, (void)num_mats;
}

void ilG_default_viewmats(void *obj, il_mat *mats, int *types, unsigned num)
{
    (void)obj, (void)mats, (void)types, (void)num;
}

void ilG_default_objmats(void *obj, const unsigned *objects, unsigned num, il_mat *out, int type)
{
    (void)obj, (void)objects, (void)num, (void)out, (void)type;
}

void ilG_default_free(void *obj)
{
    (void)obj;
}
