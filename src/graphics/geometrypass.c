#include "geometrypass.h"

#include <sys/time.h>

#include "graphics/renderer.h"
#include "graphics/bindable.h"
#include "graphics/stage.h"
#include "graphics/context.h"
#include "graphics/glutil.h"
#include "graphics/arrayattrib.h"
#include "graphics/drawable3d.h"

struct ilG_geometrypass {
    ilG_stage stage;
    IL_ARRAY(ilG_renderer*,) renderers;
};

static void draw_geometry(ilG_stage *ptr)
{
    struct ilG_geometrypass *self = (struct ilG_geometrypass*)ptr;
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

ilG_stage *ilG_geometrypass_new(ilG_context *context)
{
    struct ilG_geometrypass *self = calloc(1, sizeof(struct ilG_geometrypass));
    self->stage.context = context;
    self->stage.run = draw_geometry;
    self->stage.name = "Geometry Pass";
    return &self->stage;
}

void ilG_geometrypass_track(ilG_stage *self, ilG_renderer *renderer)
{
    IL_APPEND(((struct ilG_geometrypass*)self)->renderers, renderer);
}

