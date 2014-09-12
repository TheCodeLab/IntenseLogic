#include <uv.h>
#include <SDL.h>
#include <assert.h>

#include "tgl/tgl.h"
#include "asset/node.h"
#include "graphics/context.h"
#include "graphics/fragdata.h"
#include "graphics/graphics.h"
#include "graphics/material.h"
#include "graphics/outpass.h"
#include "graphics/renderer.h"
#include "util/timer.h"
#include "util/log.h"

typedef struct toy {
    ilG_context *context;
    ilG_matid mat;
    tgl_quad quad;
    tgl_vao vao;
    ilA_file shader;
    struct timeval start;
    /* The shadertoy.com uniforms:
      uniform vec3      iResolution;           // viewport resolution (in pixels)
      uniform float     iGlobalTime;           // shader playback time (in seconds)
      uniform float     iChannelTime[4];       // channel playback time (in seconds)
      uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
      uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
      uniform samplerXX iChannel0..3;          // input channel. XX = 2D/Cube
      uniform vec4      iDate;                 // (year, month, day, time in seconds)
      uniform float     iSampleRate;           // sound sample rate (i.e., 44100)
    */
    GLint iResolution, iGlobalTime, iMouse;
    int mouse[4];
} toy;

static void toy_draw(void *obj, ilG_rendid id)
{
    (void)id;
    toy *t = obj;
    ilG_material *mat = ilG_context_findMaterial(t->context, t->mat);
    ilG_material_bind(mat);

    glUniform2f(t->iResolution, t->context->width, t->context->height);

    struct timeval now, tv;
    timersub(&now, &t->start, &tv);
    float tf = (float)tv.tv_sec + (float)tv.tv_usec/1000000;
    glUniform1f(t->iGlobalTime, tf);

    tgl_vao_bind(&t->vao);
    tgl_quad_draw_once(&t->quad);

}

static void toy_free(void *obj)
{
    toy *t = obj;
    ilG_context_delMaterial(t->context, t->mat);
    tgl_vao_free(&t->vao);
    tgl_quad_free(&t->quad);
}

static bool toy_build(void *obj, ilG_rendid id, ilG_context *context, ilG_buildresult *out)
{
    (void)id;
    toy *t = obj;
    t->context = context;

    ilG_material m[1];
    ilG_material_init(m);
    ilG_material_name(m, "Rainbow Quad Shader");
    ilG_material_fragData(m, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_POSITION, "in_Position");
    if (!ilG_material_vertex_file(m, "id2d.vert", &out->error)) {
        return false;
    }
    ilG_material_fragment(m, t->shader);
    if (!ilG_material_link(m, context, &out->error)) {
        return false;
    }
    t->iResolution = ilG_material_getLoc(m, "iResolution");
    t->iGlobalTime = ilG_material_getLoc(m, "iGlobalTime");
    t->iMouse = ilG_material_getLoc(m, "iMouse");
    t->mat = ilG_context_addMaterial(context, *m);
    tgl_vao_init(&t->vao);
    tgl_vao_bind(&t->vao);
    tgl_quad_init(&t->quad, ILG_ARRATTR_POSITION);
    gettimeofday(&t->start, NULL);

    out->free = toy_free;
    out->update = toy_draw;
    out->obj = obj;
    return true;
}

typedef struct reload_ctx {
    uv_fs_event_t ev;
    ilG_context *context;
    ilG_matid *mat;
} reload_ctx;

void upload_cb(void *ptr)
{
    reload_ctx *ctx = ptr;
    ilG_material *mat = ilG_context_findMaterial(ctx->context, *ctx->mat);
    assert(mat);
    char *error;
    if (!ilG_material_link(mat, ctx->context, &error)) {
        il_error("Link failed: %s", error);
        free(error);
    } else {
        il_log("Shader reloaded");
    }
}

void event_cb(uv_fs_event_t *handle,
              const char *filename,
              int events,
              int status)
{
    (void)events, (void)status;
    reload_ctx *ctx = (reload_ctx*)handle;
    il_log("File updated: %s", filename);
    ilG_context_upload(ctx->context, upload_cb, ctx);
}

void *event_loop(void *ptr)
{
    uv_loop_t *loop = ptr;
    uv_run(loop, UV_RUN_DEFAULT);
    return NULL;
}

void demo_start()
{
    ilG_context context[1];
    ilG_handle out, toy_r;
    toy t[1];
    const char *shader_name = "rainbow2d.frag"; // TODO: Get shader from command line
    ilA_file shader;
    pthread_t event_thread;
    uv_loop_t loop;
    reload_ctx rctx;

    if (!ilA_fileopen(&ilG_shaders, &shader, shader_name, -1)) {
        ilA_printerror(&shader.err);
        return;
    }
    il_log("Shader path: %s", shader.name);
    memset(t, 0, sizeof(toy));
    t->shader = shader;
    ilG_context_init(context);
    context->initialTitle = "Shader Toy";
    out = ilG_build(ilG_out_builder(), context);
    toy_r = ilG_build(ilG_builder_wrap(t, toy_build), context);

    ilG_handle_addRenderer(context->root, toy_r);
    ilG_handle_addRenderer(context->root, out);

    ilG_context_start(context);

    uv_loop_init(&loop);
    memset(&rctx, 0, sizeof(reload_ctx));
    rctx.context = context;
    rctx.mat = &t->mat;
    uv_fs_event_init(&loop, &rctx.ev);
    uv_fs_event_start(&rctx.ev, event_cb, shader.name, shader.namelen);
    pthread_create(&event_thread, NULL, event_loop, &loop);

    SDL_Event ev;
    while (SDL_WaitEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            il_log("Stopping");
            ilG_context_stop(context);
            uv_stop(&loop);
            return;
        case SDL_MOUSEMOTION:
            t->mouse[0] = ev.motion.x;
            t->mouse[1] = ev.motion.y;
            if (ev.motion.state & SDL_BUTTON_LMASK) {
                t->mouse[2] = ev.motion.x;
                t->mouse[3] = ev.motion.y;
            }
            break;
        }
    }
}
