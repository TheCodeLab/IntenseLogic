#include "geometrypass.h"

#include <sys/time.h>

#include "graphics/renderer.h"
#include "graphics/context.h"
#include "graphics/transform.h"
#include "graphics/arrayattrib.h"
#include "tgl/tgl.h"

static void geometry_free(void *ptr)
{
    (void)ptr;
}

static void geometry_update(void *ptr, ilG_rendid id)
{
    (void)ptr, (void)id;
    tgl_check("Unknown");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    tgl_check("Could not setup to draw geometry");
}

static bool geometry_build(void *ptr, ilG_rendid id, ilG_context *ctx, ilG_buildresult *out)
{
    (void)ctx;
    ilG_renderman_addName(&ctx->manager, id, "Geometry");
    *out = (ilG_buildresult) {
        .free = geometry_free,
        .update = geometry_update,
        .types = NULL,
        .num_types = 0,
        .obj = ptr
    };
    return true;
}

ilG_builder ilG_geometry_builder()
{
    return ilG_builder_wrap(NULL, geometry_build);
}
