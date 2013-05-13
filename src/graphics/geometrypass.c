#include "geometrypass.h"

#include <sys/time.h>

#include "graphics/tracker.h"
#include "graphics/bindable.h"
#include "graphics/stage.h"
#include "graphics/context.h"
#include "graphics/glutil.h"

static void draw_geometry(ilG_stage *self)
{
    glClearColor(0.39, 0.58, 0.93, 1.0); // cornflower blue
    ilG_testError("glClearColor");
    glClearDepth(1.0);
    ilG_testError("glClearDepth");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ilG_testError("glClear");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    ilG_testError("glEnable");
    ilG_testError("Error setting up for draw");

    ilG_context *context = self->context;
    struct timeval tv;
    il_positionable* pos = NULL;
    ilG_trackiterator * iter = ilG_trackiterator_new(context);
    gettimeofday(&tv, NULL);
    //const ilG_bindable *drawable = NULL, *material = NULL, *texture = NULL;

    while (ilG_trackIterate(iter)) {
        pos = ilG_trackGetPositionable(iter);
        context->positionable = pos;

        ilG_bindable_swap(&context->drawableb, (void**)&context->drawable, ilG_trackGetDrawable(iter));
        ilG_bindable_swap(&context->materialb, (void**)&context->material, ilG_trackGetMaterial(iter));
        ilG_bindable_swap(&context->textureb,  (void**)&context->texture,  ilG_trackGetTexture(iter));

        ilG_bindable_action(context->materialb, context->material);
        ilG_bindable_action(context->textureb,  context->texture);
        ilG_bindable_action(context->drawableb, context->drawable);
    }
    ilG_bindable_unbind(context->materialb, context->material);
    ilG_bindable_unbind(context->textureb,  context->texture);
    ilG_bindable_unbind(context->drawableb, context->drawable);
    context->drawable = NULL;
    context->material = NULL;
    context->texture = NULL;
    context->drawableb = NULL;
    context->materialb = NULL;
    context->textureb = NULL;
}

void ilG_geometrypass(ilG_stage* self)
{
    self->run = draw_geometry;
    self->name = "Geometry Pass";
}

