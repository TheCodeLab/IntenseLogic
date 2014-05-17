#include "geometrypass.h"

#include <sys/time.h>

#include "graphics/renderer.h"
#include "graphics/bindable.h"
#include "graphics/context.h"
#include "graphics/glutil.h"
#include "graphics/arrayattrib.h"
#include "graphics/drawable3d.h"
#include "util/ilassert.h"

static void geometry_free(void *ptr, ilG_rendid id)
{
    (void)ptr, (void)id;
}

static void geometry_draw(void *ptr, ilG_rendid id)
{
    (void)ptr, (void)id;
    ilG_testError("Unknown");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    ilG_testError("Could not setup to draw geometry");
}

static bool geometry_build(void *ptr, ilG_rendid id, ilG_context *ctx, ilG_renderer *out)
{
    (void)ctx;
    ilG_context_addName(ctx, id, "Geometry");
    *out = (ilG_renderer) {
        .id = id,
        .free = geometry_free,
        .draw = geometry_draw,
        .obj = ptr
    };
    return true;
}

ilG_builder ilG_geometry_builder()
{
    return ilG_builder_wrap(NULL, geometry_build);
}

