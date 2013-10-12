#include "transparencypass.h"

#include "graphics/stage.h"
#include "graphics/context.h"
#include "graphics/tracker.h"
#include "graphics/bindable.h"
#include "graphics/drawable3d.h"
#include "graphics/arrayattrib.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "common/positionable.h"

struct rinfo {
    il_positionable *positionable;
    ilG_drawable3d *drawable;
    ilG_material *material;
    ilG_texture *texture;
};

static void draw_transparency(ilG_stage *self)
{
    ilG_testError("Unknown");
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    ilG_testError("glEnable");

    ilG_context *context = self->context;
    ilG_trackiterator *iter = ilG_trackiterator_new(context);
    ilG_drawable3d *drawable = NULL;
    IL_ARRAY(struct rinfo,) zindex = {0,0,0};
    unsigned i;

    while (ilG_trackIterate(iter)) {
        drawable = ilG_trackGetDrawable(iter);
        if (ILG_TESTATTR(drawable->attrs, ILG_ARRATTR_ISTRANSPARENT)) {
            struct rinfo rinfo = (struct rinfo){ilG_trackGetPositionable(iter), drawable, ilG_trackGetMaterial(iter), ilG_trackGetTexture(iter)};
            IL_APPEND(zindex, rinfo);
        }
    }
    // TODO: Z-sort
    for (i = 0; i < zindex.length; i++) {
        struct rinfo *rinfo = &zindex.data[i];
        context->positionable = rinfo->positionable;

        ilG_bindable_swap(&context->drawableb, (void**)&context->drawable, rinfo->drawable);
        ilG_bindable_swap(&context->materialb, (void**)&context->material, rinfo->material);
        ilG_bindable_swap(&context->textureb,  (void**)&context->texture,  rinfo->texture);

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

void ilG_transparencypass(ilG_stage *self)
{
    self->run = draw_transparency;
    self->name = "Transparency Pass";
}

