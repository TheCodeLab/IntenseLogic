#include <SDL.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include "graphics/arrayattrib.h"
#include "graphics/context.h"
#include "graphics/floatspace.h"
#include "graphics/material.h"
#include "tgl/tgl.h"
#include "graphics/renderer.h"
#include "math/matrix.h"
#include "util/log.h"

struct quad {
    ilG_renderman *rm;
    ilG_matid mat;
    tgl_quad quad;
    tgl_vao vao;
};

static void quad_draw(void *obj, ilG_rendid id)
{
    (void)id;
    struct quad *q = obj;
    ilG_material *mat = ilG_renderman_findMaterial(q->rm, q->mat);
    ilG_material_bind(mat);
    tgl_vao_bind(&q->vao);
    tgl_quad_draw_once(&q->quad);
}

static void quad_free(void *obj)
{
    struct quad *q = obj;
    ilG_renderman_delMaterial(q->rm, q->mat);
    tgl_vao_free(&q->vao);
    tgl_quad_free(&q->quad);
    free(q);
}

static bool quad_build(void *obj, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    struct quad *q = obj;
    q->rm = rm;

    ilG_material m;
    ilG_material_init(&m);
    ilG_material_name(&m, "Rainbow Quad Shader");
    ilG_material_arrayAttrib(&m, ILG_ARRATTR_POSITION, "in_Position");
    if (!ilG_renderman_addMaterialFromFile(rm, m, "id2d.vert", "rainbow2d.frag", &q->mat, &out->error)) {
        return false;
    }
    tgl_vao_init(&q->vao);
    tgl_vao_bind(&q->vao);
    tgl_quad_init(&q->quad, ILG_ARRATTR_POSITION);

    out->free = quad_free;
    out->update = quad_draw;
    out->obj = obj;
    out->name = strdup("Quad");
    return true;
}

static ilG_builder quad_builder()
{
    struct quad *q = calloc(1, sizeof(struct quad));

    return ilG_builder_wrap(q, quad_build);
}

void demo_start()
{
    ilG_context *context = ilG_context_new();
    ilG_context_hint(context, ILG_CONTEXT_DEBUG_RENDER, 1);
    ilG_handle out, quad;
    out = ilG_build(ilG_out_builder(context, NULL, NULL), &context->manager);
    quad = ilG_build(quad_builder(), &context->manager);

    ilG_handle_addRenderer(context->root, quad);
    ilG_handle_addRenderer(context->root, out);

    context->initialTitle = strdup("Quad Demo");
    ilG_context_start(context);

    SDL_Event ev;
    while (SDL_WaitEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            il_log("Stopping");
            ilG_context_stop(context);
            return;
        }
    }
}
