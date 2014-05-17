#include "transparencypass.h"

#include "graphics/renderer.h"
#include "graphics/context.h"
#include "graphics/bindable.h"
#include "graphics/drawable3d.h"
#include "graphics/arrayattrib.h"
#include "graphics/material.h"
#include "common/world.h"
#include "util/array.h"
#include "util/log.h"

static void trans_free(void *ptr, ilG_rendid id)
{
    (void)ptr, (void)id;
}

static void trans_draw(void *ptr, ilG_rendid id)
{
    (void)ptr, (void)id;
    ilG_testError("Unknown");
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    ilG_testError("glEnable");
}

static bool trans_build(void *ptr, ilG_rendid id, ilG_context *context, ilG_renderer *out)
{
    (void)context;
    *out = (ilG_renderer) {
        .id = id,
        .free = trans_free,
        .draw = trans_draw,
        .obj = ptr
    };
    return true;
}

ilG_builder ilG_transparency_builder()
{
    return ilG_builder_wrap(NULL, trans_build);
}


