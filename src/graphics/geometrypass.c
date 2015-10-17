#include "graphics/renderer.h"

#include <sys/time.h>

#include "graphics/transform.h"
#include "graphics/arrayattrib.h"
#include "tgl/tgl.h"

void ilG_geometry_bind(tgl_fbo *gbuffer)
{
    static const unsigned order[] = {
        ILG_GBUFFER_ALBEDO,
        ILG_GBUFFER_NORMAL,
        ILG_GBUFFER_REFRACTION,
        ILG_GBUFFER_GLOSS,
        ILG_GBUFFER_EMISSION,
    };
    tgl_fbo_bind_with(gbuffer, TGL_FBO_RW, 5, order);
    glClearDepth(1.0);
    glClearColor(0,0,0,0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
}

static void geometry_update(void *ptr, ilG_rendid id)
{
    (void)id;
    ilG_renderman *rm = ptr;
    tgl_check("Unknown");
    ilG_geometry_bind(&rm->gbuffer);
    tgl_check("Could not setup to draw geometry");
}

static bool geometry_build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id, (void)rm;
    *out = (ilG_buildresult) {
        .update = geometry_update,
        .types = NULL,
        .num_types = 0,
        .obj = ptr,
        .name = strdup("Geometry")
    };
    return true;
}

ilG_builder ilG_geometry_builder(ilG_renderman *rm)
{
    return ilG_builder_wrap(rm, geometry_build);
}
