#include <SDL2/SDL.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include "graphics/arrayattrib.h"
#include "graphics/context.h"
#include "graphics/floatspace.h"
#include "graphics/fragdata.h"
#include "graphics/geometrypass.h"
#include "graphics/glutil.h"
#include "graphics/material.h"
#include "graphics/outpass.h"
#include "graphics/quad.h"
#include "graphics/renderer.h"
#include "math/matrix.h"
#include "util/log.h"

struct quad {
    ilG_material mat;
    ilG_quad *quad;
};

static void quad_draw(void *obj, ilG_rendid id)
{
    (void)id;
    struct quad *q = obj;
    ilG_material_bind(&q->mat);
    ilG_quad_draw(q->quad);
}

static void quad_free(void *obj)
{
    struct quad *q = obj;
    ilG_material_free(&q->mat);
    free(q);
}

static bool quad_build(void *obj, ilG_rendid id, ilG_context *context, ilG_buildresult *out)
{
    (void)id;
    struct quad *q = obj;
    if (ilG_material_link(&q->mat, context)) {
        return false;
    }

    memset(out, 0, sizeof(ilG_buildresult));
    out->free = quad_free;
    out->update = quad_draw;
    out->obj = obj;
    return true;
}

static ilG_builder quad_builder()
{
    struct quad *q = calloc(1, sizeof(struct quad));
    ilG_material_init(&q->mat);
    ilG_material_vertex_file(&q->mat, "id2d.vert");
    ilG_material_fragment_file(&q->mat, "rainbow2d.frag");
    ilG_material_name(&q->mat, "Rainbow Quad Shader");
    ilG_material_fragData(&q->mat, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    ilG_material_arrayAttrib(&q->mat, ILG_ARRATTR_POSITION, "in_Position");

    return ilG_builder_wrap(q, quad_build);
}

void demo_start()
{
    ilG_context *context = ilG_context_new();
    ilG_context_hint(context, ILG_CONTEXT_DEBUG_RENDER, 1);
    ilG_handle out, quad;
    out = ilG_build(ilG_out_builder(), context);
    quad = ilG_build(quad_builder(), context);

    ilG_handle_addRenderer(context->root, quad);
    ilG_handle_addRenderer(context->root, out);

    ilG_context_rename(context, "Quad Demo");
    ilG_context_start(context);

    SDL_Event ev;
    while (1) {
        if (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                il_log("Stopping");
                ilG_context_stop(context);
                return;
            }
        }
        struct timespec t = {0, 50 * 1000 * 1000};
        nanosleep(&t, &t);
    }
}
