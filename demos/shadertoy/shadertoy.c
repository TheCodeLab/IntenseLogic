#include <uv.h>
#include <SDL.h>
#include <assert.h>

#include "tgl/tgl.h"
#include "asset/node.h"
#include "graphics/context.h"
#include "graphics/fragdata.h"
#include "graphics/graphics.h"
#include "graphics/material.h"
#include "graphics/renderer.h"
#include "util/timer.h"
#include "util/log.h"

typedef struct toy {
    ilG_context *context;
    ilG_renderman *rm;
    ilG_matid mat;
    tgl_quad quad;
    tgl_vao vao;
    ilA_file shader;
    struct timeval start_real;
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
    bool linked, paused;
    float mono_last, mono_start, speed;
} toy;

static void toy_draw(void *obj, ilG_rendid id)
{
    (void)id;
    toy *t = obj;
    if (!t->linked) {
        return;
    }
    ilG_material *mat = ilG_renderman_findMaterial(t->rm, t->mat);
    ilG_material_bind(mat);

    glUniform2f(t->iResolution, t->context->width, t->context->height);

    struct timeval now, tv;
    float tf;
    if (t->paused) {
        tf = t->mono_last;
    } else {
        gettimeofday(&now, NULL);
        timersub(&now, &t->start_real, &tv);
        t->start_real = now;
        t->mono_start += ((float)tv.tv_sec + (float)tv.tv_usec/1000000.0) * t->speed;
        tf = t->mono_last = t->mono_start;
    }
    glUniform1f(t->iGlobalTime, tf);

    glUniform4iv(t->iMouse, 1, t->mouse);

    tgl_vao_bind(&t->vao);
    tgl_quad_draw_once(&t->quad);

}

static void toy_free(void *obj)
{
    toy *t = obj;
    ilG_renderman_delMaterial(t->rm, t->mat);
    tgl_vao_free(&t->vao);
    tgl_quad_free(&t->quad);
}

static bool compile(toy *t, char **error)
{
    ilG_material *mat = ilG_renderman_findMaterial(t->rm, t->mat);
    ilG_shader *vert = ilG_renderman_findShader(t->rm, mat->vert);
    ilG_shader *frag = ilG_renderman_findShader(t->rm, mat->frag);
    if (ilG_shader_compile(frag, GL_FRAGMENT_SHADER, error)
        && ilG_material_link(mat, vert, frag, error)) {
        t->linked = true;
        t->iResolution = ilG_material_getLoc(mat, "iResolution");
        t->iGlobalTime = ilG_material_getLoc(mat, "iGlobalTime");
        t->iMouse = ilG_material_getLoc(mat, "iMouse");
        return true;
    }
    return false;
}

static bool toy_build(void *obj, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    toy *t = obj;
    t->rm = rm;

    t->speed = 1.0;
    ilG_material m[1];
    ilG_material_init(m);
    ilG_material_name(m, "Shader Toy");
    ilG_material_fragData(m, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    ilG_material_arrayAttrib(m, ILG_ARRATTR_POSITION, "in_Position");
    ilG_shader vert, frag;
    if (!ilG_shader_file(&vert, "id2d.vert", &out->error)) {
        return false;
    }
    if (!ilG_shader_compile(&vert, GL_VERTEX_SHADER, &out->error)) {
        return false;
    }
    ilG_shader_load(&frag, t->shader);
    m->vert = ilG_renderman_addShader(rm, vert);
    m->frag = ilG_renderman_addShader(rm, frag);
    t->mat = ilG_renderman_addMaterial(t->rm, *m);
    char *error;
    if (!compile(t, &error)) {
        il_error("%s", error);
        free(error);
    }
    tgl_vao_init(&t->vao);
    tgl_vao_bind(&t->vao);
    tgl_quad_init(&t->quad, ILG_ARRATTR_POSITION);
    gettimeofday(&t->start_real, NULL);

    out->free = toy_free;
    out->update = toy_draw;
    out->obj = obj;
    return true;
}

typedef struct reload_ctx {
    uv_fs_event_t ev;
    ilG_context *context;
    toy *t;
} reload_ctx;

void upload_cb(void *ptr)
{
    reload_ctx *ctx = ptr;
    toy *t = ctx->t;
    char *error;
    if (!compile(t, &error)) {
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
    ilG_renderman_upload(&ctx->context->manager, upload_cb, ctx);
}

void *event_loop(void *ptr)
{
    uv_loop_t *loop = ptr;
    uv_run(loop, UV_RUN_DEFAULT);
    return NULL;
}

extern const char *demo_shader;
void demo_start()
{
    ilG_context context[1];
    ilG_handle out, toy_r;
    toy t[1];
    ilA_file shader;
    pthread_t event_thread;
    uv_loop_t loop;
    reload_ctx rctx;

    if (!ilA_fileopen(&ilG_shaders, &shader, demo_shader, -1)) {
        ilA_printerror(&shader.err);
        return;
    }
    il_log("Shader path: %s", shader.name);
    memset(t, 0, sizeof(toy));
    t->shader = shader;
    ilG_context_init(context);
    ilG_context_hint(context, ILG_CONTEXT_VSYNC, 1);
    context->initialTitle = "Shader Toy";
    t->context = context;
    out = ilG_build(ilG_out_builder(context, NULL, NULL), &context->manager);
    toy_r = ilG_build(ilG_builder_wrap(t, toy_build), &context->manager);

    ilG_handle_addRenderer(context->root, toy_r);
    ilG_handle_addRenderer(context->root, out);

    ilG_context_start(context);

    uv_loop_init(&loop);
    memset(&rctx, 0, sizeof(reload_ctx));
    rctx.context = context;
    rctx.t = t;
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
        case SDL_KEYDOWN:
            if (ev.key.state != SDL_PRESSED) {
                break;
            }
            switch (ev.key.keysym.sym) {
            case SDLK_r:
                il_log("Replay");
                gettimeofday(&t->start_real, NULL);
                t->mono_start = 0.0;
                t->mono_last = 0.0;
                break;
            case SDLK_p:
                t->paused = !t->paused;
                il_log("%s", t->paused? "Paused" : "Unpaused");
                gettimeofday(&t->start_real, NULL);
                break;
            case SDLK_LEFT:
                t->speed /= 2;
                il_log("Speed: %f", t->speed);
                break;
            case SDLK_RIGHT:
                t->speed *= 2;
                il_log("Speed: %f", t->speed);
                break;
            }
            break;
        }
    }
}
