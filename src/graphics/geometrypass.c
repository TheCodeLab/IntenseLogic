#include "geometrypass.h"

#include <sys/time.h>

#include "graphics/renderer.h"
#include "graphics/bindable.h"
#include "graphics/stage.h"
#include "graphics/context.h"
#include "graphics/glutil.h"
#include "graphics/arrayattrib.h"
#include "graphics/drawable3d.h"
#include "util/ilassert.h"

struct ilG_geometry {
    IL_ARRAY(ilG_renderer*,) renderers;
};

static void geometry_run(void *ptr)
{
    ilG_geometry *self = ptr;
    ilG_testError("Unknown");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    ilG_testError("glEnable");
    
    unsigned i;
    for (i = 0; i < self->renderers.length; i++) {
        ilG_renderer_draw(self->renderers.data[i]);
        ilG_testError("Rendering %s", ilG_renderer_getName(self->renderers.data[i]));
    }
}

static int geometry_track(void *self, ilG_renderer *renderer)
{
    il_return_val_on_fail(ilG_renderer_isComplete(renderer), 0);
    IL_APPEND(((ilG_geometry*)self)->renderers, renderer);
    return 1;
}

const ilG_stagable ilG_geometry_stage = {
    .run = geometry_run,
    .track = geometry_track,
    .name = "Geometry Pass"
};

ilG_geometry *ilG_geometry_new(ilG_context *context)
{
    (void)context;
    ilG_geometry *self = calloc(1, sizeof(ilG_geometry));
    return self;
}


