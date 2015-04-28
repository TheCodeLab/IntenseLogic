#include "graphics/renderer.h"

#include <sys/time.h>

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
    (void)id;
    ilG_context *context = ptr;
    tgl_check("Unknown");
    static const unsigned order[] = {
        ILG_CONTEXT_ALBEDO,
        ILG_CONTEXT_NORMAL,
        ILG_CONTEXT_REFLECT,
        ILG_CONTEXT_GLOSS,
        ILG_CONTEXT_EMISSION,
    };
    tgl_fbo_bind_with(&context->gbuffer, TGL_FBO_RW, 5, order);
    glClearDepth(1.0);
    glClearColor(0,0,0,0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    tgl_check("Could not setup to draw geometry");
}

static bool geometry_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id, (void)rm;
    *out = (ilG_buildresult) {
        .free = geometry_free,
        .update = geometry_update,
        .types = NULL,
        .num_types = 0,
        .obj = ptr,
        .name = strdup("Geometry")
    };
    return true;
}

ilG_builder ilG_geometry_builder(ilG_context *context)
{
    return ilG_builder_wrap(context, geometry_build);
}
