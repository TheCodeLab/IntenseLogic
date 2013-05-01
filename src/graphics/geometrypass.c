#include "geometrypass.h"

#include <sys/time.h>

#include "graphics/tracker.h"
#include "graphics/bindable.h"
#include "graphics/stage.h"
#include "graphics/context.h"
#include "graphics/glutil.h"

static void draw_geometry(ilG_stage *self)
{
    glClearColor(1.0, 0.41, 0.72, 1.0); // hot pink because why not
    ilG_testError("glClearColor");
    glClearDepth(1.0);
    ilG_testError("glClearDepth");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ilG_testError("glClear");
    glEnable(GL_CULL_FACE);
    ilG_testError("glEnable");
    ilG_testError("Error setting up for draw");

    ilG_context *context = self->context;
    struct timeval tv;
    il_positionable* pos = NULL;
    ilG_trackiterator * iter = ilG_trackiterator_new(context);
    gettimeofday(&tv, NULL);
    const ilG_bindable *drawable = NULL, *material = NULL, *texture = NULL;

    while (ilG_trackIterate(iter)) {
        pos = ilG_trackGetPositionable(iter);
        context->positionable = pos;

        ilG_bindable_swap(&drawable, (void**)&context->drawable, ilG_trackGetDrawable(iter));
        ilG_bindable_swap(&material, (void**)&context->material, ilG_trackGetMaterial(iter));
        ilG_bindable_swap(&texture,  (void**)&context->texture,  ilG_trackGetTexture(iter));

        ilG_bindable_action(material, context->material);
        ilG_bindable_action(texture,  context->texture);
        ilG_bindable_action(drawable, context->drawable);
    }
    context->drawable = NULL;
    context->material = NULL;
    context->texture = NULL;
}

void ilG_geometrypass(ilG_stage* self)
{
    self->run = draw_geometry;
    self->name = "Geometry Pass";
}

