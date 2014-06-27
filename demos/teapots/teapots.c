#include <SDL2/SDL.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include "graphics/context.h"
#include "graphics/floatspace.h"
#include "graphics/fragdata.h"
#include "graphics/geometrypass.h"
#include "graphics/glutil.h"
#include "graphics/lightpass.h"
#include "graphics/material.h"
#include "graphics/mesh.h"
#include "graphics/outpass.h"
#include "graphics/renderer.h"
#include "graphics/tex.h"
#include "math/matrix.h"
#include "util/log.h"

#ifndef M_PI
#define M_PI 3.1415926535
#endif

struct teapot {
    ilG_material mat;
    ilG_mesh mesh;
    ilG_tex tex;
    GLuint mvp_loc;
};

static void teapot_draw(void *obj, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats)
{
    (void)id, (void)objects;
    struct teapot *t = obj;
    ilG_material_bind(&t->mat);
    ilG_mesh_bind(&t->mesh);
    ilG_tex_bind(&t->tex);
    for (unsigned i = 0; i < num_mats; i++) {
        ilG_material_bindMatrix(&t->mat, t->mvp_loc, mats[0][i]);
        ilG_mesh_draw(&t->mesh);
    }
}

static void teapot_free(void *obj)
{
    struct teapot *t = obj;
    ilG_material_free(&t->mat);
    ilG_mesh_free(&t->mesh);
    free(t);
}

static bool teapot_build(void *obj, ilG_rendid id, ilG_context *context, ilG_buildresult *out)
{
    (void)id;
    struct teapot *t = obj;
    if (ilG_material_link(&t->mat, context)) {
        return false;
    }
    t->mvp_loc = ilG_material_getLoc(&t->mat, "mvp");

    if (!ilG_mesh_build(&t->mesh, context)) {
        return false;
    }

    ilG_tex_build(&t->tex, context);

    memset(out, 0, sizeof(ilG_buildresult));
    int *types = calloc(1, sizeof(int));
    types[0] = ILG_MVP;
    out->free = teapot_free;
    out->draw = teapot_draw;
    out->types = types;
    out->num_types = 1;
    out->obj = obj;
    return true;
}

ilG_builder teapot_builder()
{
    struct teapot *t = calloc(1, sizeof(struct teapot));
    ilG_material *m = &t->mat;
    ilG_material_init(m);
    ilG_material_vertex_file(m, "test.vert");
    ilG_material_fragment_file(m, "test.frag");
    ilG_material_name(m, "Teapot Material");
    ilG_material_fragData(m, ILG_FRAGDATA_NORMAL, "out_Normal");
    ilG_material_fragData(m, ILG_FRAGDATA_ACCUMULATION, "out_Ambient");
    ilG_material_fragData(m, ILG_FRAGDATA_DIFFUSE, "out_Diffuse");
    ilG_material_fragData(m, ILG_FRAGDATA_SPECULAR, "out_Specular");
    ilG_material_arrayAttrib(m, ILG_MESH_POS, "in_Position");
    ilG_material_arrayAttrib(m, ILG_MESH_TEX, "in_Texcoord");
    ilG_material_arrayAttrib(m, ILG_MESH_NORM, "in_Normal");
    ilG_material_arrayAttrib(m, ILG_MESH_DIFFUSE, "in_Diffuse");
    ilG_material_arrayAttrib(m, ILG_MESH_SPECULAR, "in_Specular");
    ilG_material_textureUnit(m, 0, "tex");

    ilG_mesh_fromfile(&t->mesh, "demos/teapots/teapot.obj");
    ilG_tex_loadfile(&t->tex, "demos/teapots/white-marble-texture.png");

    return ilG_builder_wrap(t, teapot_build);
}

static void update_camera(const il_value *data, il_value *ctx)
{
    (void)data;
    ilG_floatspace *fs = il_value_tomvoid(ctx);
    struct timeval ts;
    gettimeofday(&ts, NULL);
    int secs = 5;
    float delta = ((float)(ts.tv_sec%secs) + ts.tv_usec / 1000000.0) / secs;
    il_vec3 v;
    int scale = 20;
    v.x = sinf(delta * M_PI * 2) * scale;
    v.y = 0;
    v.z = cosf(delta * M_PI * 2) * scale;
    il_quat q = il_quat_fromAxisAngle(0, 1, 0, delta * M_PI * 2);
    il_positionable_setPosition(&fs->camera, v);
    il_positionable_setRotation(&fs->camera, q);
}

void demo_start()
{
    ilG_context *context = ilG_context_new();
    ilG_context_hint(context, ILG_CONTEXT_DEBUG_RENDER, 1);
    ilG_handle geom, lights, out, teapot;
    geom = ilG_build(ilG_geometry_builder(), context);
    lights = ilG_build(ilG_lights_builder(), context);
    out = ilG_build(ilG_out_builder(), context);
    teapot = ilG_build(teapot_builder(), context);

    ilG_handle_addRenderer(geom, teapot);
    ilG_handle_addRenderer(context->root, geom);
    ilG_handle_addRenderer(context->root, lights);
    ilG_handle_addRenderer(context->root, out);

    ilG_floatspace *fs = ilG_floatspace_new(il_world_new(10));
    ilG_floatspace_build(fs, context);
    fs->projection = il_mat_perspective(M_PI / 4.0, 4.0/3, .5, 200);

    il_positionable_setPosition(&fs->camera, il_vec3_new(0, 0, 20));

    il_positionable pos = il_positionable_new(fs->world);
    il_positionable_setPosition(&pos, il_vec3_new(0, -4, 0));
    ilG_floatspace_addPos(fs, teapot, pos);

    il_positionable lightp = il_positionable_new(fs->world);
    il_positionable_setPosition(&lightp, il_vec3_new(20, 3, 20));
    ilG_floatspace_addPos(fs, lights, lightp);

    ilG_light lightl;
    lightl.color = il_vec3_new(.8, .7, .2);
    lightl.radius = 50;
    ilG_handle_addLight(lights, lightl);

    il_storage_void sv = {fs, NULL};
    ilE_register(context->tick, ILE_DONTCARE, ILE_ANY, update_camera, il_value_opaque(sv));

    ilG_context_rename(context, "Teapots Demo");
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