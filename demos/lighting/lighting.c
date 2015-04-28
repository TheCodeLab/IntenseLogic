#include <SDL.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include "asset/node.h"
#include "graphics/graphics.h"
#include "graphics/context.h"
#include "graphics/floatspace.h"
#include "graphics/material.h"
#include "graphics/mesh.h"
#include "graphics/renderer.h"
#include "graphics/tex.h"
#include "graphics/transform.h"
#include "math/matrix.h"
#include "util/log.h"

#include "../helper.h"

#ifndef M_PI
#define M_PI 3.1415926535
#endif

extern ilA_fs demo_fs;

ilG_builder ilG_computer_builder();

static void update_camera(const il_value *data, il_value *ctx)
{
    (void)data;
    ilG_floatspace *fs = il_value_tomvoid(ctx);
    struct timeval ts;
    gettimeofday(&ts, NULL);
    int secs = 10;
    const float dist = 10.0;
    float delta = ((float)(ts.tv_sec%secs) + ts.tv_usec / 1000000.0) / secs;
    il_vec3 v;
    v.x = sinf(delta * M_PI * 2) * dist;
    v.y = 0;
    v.z = cosf(delta * M_PI * 2) * dist;
    il_quat q = il_quat_fromAxisAngle(0, 1, 0, delta * M_PI * 2);
    il_pos_setPosition(&fs->camera, v);
    il_pos_setRotation(&fs->camera, q);
}

void demo_start()
{
    ilA_adddir(&demo_fs, "demos/lighting/", -1);
    ilA_adddir(&ilG_shaders, "demos/lighting/", -1);

    helper_config conf;
    //conf.sky = true;
    conf.geom = true;
    conf.lights = true;
    conf.out = true;
    conf.name = "Lighting Demo";
    helper_result h = helper_create(&conf);
    ilG_context *context = h.context;
    ilG_context_hint(context, ILG_CONTEXT_HDR, 1);
    ilG_context_hint(context, ILG_CONTEXT_VSYNC, 1);

    ilG_handle comp = ilG_build(ilG_computer_builder(), &context->manager);
    ilG_handle_addRenderer(h.geom, comp);

    *h.ambient_col = il_vec3_new(0.1, 0.1, 0.1);

    ilG_floatspace fs;
    ilG_floatspace_init(&fs, 2);
    ilG_floatspace_build(&fs, context);
    fs.projection = il_mat_perspective(M_PI / 4.0, 4.0/3, .5, 200);

    il_pos_setPosition(&fs.camera, il_vec3_new(0, 0, 20));

    il_pos compp = il_pos_new(&fs);
    ilG_floatspace_addPos(&fs, comp, compp);

    il_pos lightp = il_pos_new(&fs);
    il_pos_setPosition(&lightp, il_vec3_new(20, 3, 20));
    ilG_floatspace_addPos(&fs, h.lights, lightp);

    ilG_light lightl;
    lightl.color = il_vec3_new(.8*2, .7*2, .2*2);
    lightl.radius = 50;
    ilG_handle_addLight(h.lights, lightl);

    il_storage_void sv = {&fs, NULL};
    ilE_register(&context->tick, ILE_DONTCARE, ILE_ANY, update_camera, il_value_opaque(sv));

    context->initialTitle = strdup("Lighting Demo");
    ilG_context_start(context);

    SDL_Event ev;
    while (SDL_WaitEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            il_log("Stopping");
            ilG_context_end(context);
            return;
        }
    }
}
