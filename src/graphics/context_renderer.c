#include "graphics/context.h"

#include "math/matrix.h"

bool ilG_context_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)ptr, (void)id, (void)rm;
    out->name = strdup("Context");
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
