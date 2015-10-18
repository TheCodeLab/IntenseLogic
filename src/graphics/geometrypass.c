#include "graphics/renderer.h"

#include <sys/time.h>

#include "graphics/transform.h"
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
