#include <SDL2/SDL.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include "graphics/arrayattrib.h"
#include "graphics/context.h"
#include "graphics/floatspace.h"
#include "graphics/fragdata.h"
#include "graphics/geometrypass.h"
#include "graphics/material.h"
#include "graphics/outpass.h"
#include "tgl/tgl.h"
#include "graphics/renderer.h"
#include "math/matrix.h"
#include "util/log.h"

struct quad {
    ilG_context *context;
    ilG_matid mat;
    tgl_quad quad;
    tgl_vao vao;
};

static void quad_draw(void *obj, ilG_rendid id)
{
    (void)id;
    struct quad *q = obj;
    ilG_material *mat = ilG_context_findMaterial(q->context, q->mat);
    ilG_material_bind(mat);
    tgl_vao_bind(&q->vao);
    tgl_quad_draw_once(&q->quad);
}

static void quad_free(void *obj)
{
    struct quad *q = obj;
    ilG_context_delMaterial(q->context, q->mat);
    tgl_vao_free(&q->vao);
    tgl_quad_free(&q->quad);
    free(q);
}

static bool quad_build(void *obj, ilG_rendid id, ilG_context *context, ilG_buildresult *out)
{
    (void)id;
    struct quad *q = obj;
    q->context = context;

    ilG_material m;
    ilG_material_init(&m);
    ilG_material_name(&m, "Rainbow Quad Shader");
    ilG_material_fragData(&m, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    ilG_material_arrayAttrib(&m, ILG_ARRATTR_POSITION, "in_Position");
    if (!ilG_material_vertex_file(&m, "id2d.vert", &out->error)) {
        return false;
    }
    if (!ilG_material_fragment_file(&m, "rainbow2d.frag", &out->error)) {
        return false;
    }
    if (!ilG_material_link(&m, context, &out->error)) {
        return false;
    }
    q->mat = ilG_context_addMaterial(context, m);
    tgl_vao_init(&q->vao);
    tgl_vao_bind(&q->vao);
    tgl_quad_init(&q->quad, ILG_ARRATTR_POSITION);

    memset(out, 0, sizeof(ilG_buildresult));
    out->free = quad_free;
    out->update = quad_draw;
    out->obj = obj;
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
    out = ilG_build(ilG_out_builder(), context);
    quad = ilG_build(quad_builder(), context);

    ilG_handle_addRenderer(context->root, quad);
    ilG_handle_addRenderer(context->root, out);

    ilG_context_rename(context, "Quad Demo");
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
