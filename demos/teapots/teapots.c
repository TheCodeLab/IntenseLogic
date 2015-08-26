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

#ifndef M_PI
#define M_PI 3.1415926535
#endif

typedef struct teapot {
    ilG_renderman *rm;
    ilG_matid mat;
    ilG_mesh mesh;
    ilG_tex tex;
    GLuint mvp_loc, imt_loc;
} teapot;

extern ilA_fs demo_fs;

static void teapot_draw(void *obj, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats)
{
    (void)id, (void)objects;
    teapot *t = obj;
    ilG_material *mat = ilG_renderman_findMaterial(t->rm, t->mat);
    ilG_material_bind(mat);
    ilG_mesh_bind(&t->mesh);
    ilG_tex_bind(&t->tex);
    for (unsigned i = 0; i < num_mats; i++) {
        ilG_material_bindMatrix(mat, t->mvp_loc, mats[0][i]);
        ilG_material_bindMatrix(mat, t->imt_loc, mats[1][i]);
        ilG_mesh_draw(&t->mesh);
    }
}

static void teapot_free(void *obj)
{
    teapot *t = obj;
    ilG_renderman_delMaterial(t->rm, t->mat);
    ilG_mesh_free(&t->mesh);
    free(t);
}

static bool teapot_build(void *obj, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    teapot *t = obj;
    t->rm = rm;

    ilG_material m;
    ilG_material_init(&m);
    ilG_material_name(&m, "Teapot Material");
    ilG_material_arrayAttrib(&m, ILG_MESH_POS, "in_Position");
    ilG_material_arrayAttrib(&m, ILG_MESH_TEX, "in_Texcoord");
    ilG_material_arrayAttrib(&m, ILG_MESH_NORM, "in_Normal");
    ilG_material_arrayAttrib(&m, ILG_MESH_DIFFUSE, "in_Diffuse");
    ilG_material_arrayAttrib(&m, ILG_MESH_SPECULAR, "in_Specular");
    ilG_material_textureUnit(&m, 0, "tex");
    ilG_material_fragData(&m, ILG_CONTEXT_ALBEDO, "out_Albedo");
    ilG_material_fragData(&m, ILG_CONTEXT_NORMAL, "out_Normal");
    ilG_material_fragData(&m, ILG_CONTEXT_REFRACTION, "out_Refraction");
    ilG_material_fragData(&m, ILG_CONTEXT_GLOSS, "out_Gloss");
    if (!ilG_renderman_addMaterialFromFile(rm, m, "teapot.vert", "teapot.frag", &t->mat, &out->error)) {
        return false;
    }
    ilG_material *mat = ilG_renderman_findMaterial(rm, t->mat);
    t->mvp_loc = ilG_material_getLoc(mat, "mvp");
    t->imt_loc = ilG_material_getLoc(mat, "imt");

    if (!ilG_mesh_fromfile(&t->mesh, &demo_fs, "teapot.obj")) {
        return false;
    }
    if (!ilG_mesh_build(&t->mesh)) {
        return false;
    }

    ilG_tex_build(&t->tex);

    int *types = calloc(2, sizeof(int));
    types[0] = ILG_MVP;
    types[1] = ILG_IMT;
    out->free = teapot_free;
    out->draw = teapot_draw;
    out->types = types;
    out->num_types = 2;
    out->obj = obj;
    out->name = strdup("Teapot");
    return true;
}

ilG_builder teapot_builder()
{
    struct teapot *t = calloc(1, sizeof(struct teapot));

    ilG_tex_loadfile(&t->tex, &demo_fs, "white-marble-texture.png");
    t->tex.unit = 0;

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
    il_pos_setPosition(&fs->camera, v);
    il_pos_setRotation(&fs->camera, q);
}

void demo_start()
{
    ilA_adddir(&demo_fs, "demos/teapots/", -1);
    ilA_adddir(&ilG_shaders, "demos/teapots/", -1);
    ilG_context *context = ilG_context_new();
    ilG_context_hint(context, ILG_CONTEXT_DEBUG_RENDER, 1);
    ilG_context_hint(context, ILG_CONTEXT_VSYNC, 1);
    ilG_handle geom, ambient, lights, out, teapot;
    ilG_renderman *rm = &context->manager;
    il_vec3 ambient_col = il_vec3_new(0.5, 0.5, 0.5);
    geom = ilG_build(ilG_geometry_builder(context), rm);
    lights = ilG_build(ilG_sunlight_builder(context), rm);
    ambient = ilG_build(ilG_ambient_builder(context, &ambient_col), rm);
    out = ilG_build(ilG_out_builder(context, NULL, NULL), rm);
    teapot = ilG_build(teapot_builder(), rm);

    ilG_handle_addRenderer(context->root, geom);
    ilG_handle_addRenderer(geom, teapot);
    ilG_handle_addRenderer(context->root, ambient);
    ilG_handle_addRenderer(context->root, lights);
    ilG_handle_addRenderer(context->root, out);

    ilG_floatspace fs;
    ilG_floatspace_init(&fs, 2);
    ilG_floatspace_build(&fs, context);
    fs.projection = il_mat_perspective(M_PI / 4.0, 4.0/3, .5, 200);

    il_pos_setPosition(&fs.camera, il_vec3_new(0, 0, 20));

    il_pos pos = il_pos_new(&fs);
    il_pos_setPosition(&pos, il_vec3_new(0, -4, 0));
    ilG_floatspace_addPos(&fs, teapot, pos);

    il_pos lightp = il_pos_new(&fs);
    il_pos_setPosition(&lightp, il_vec3_new(20, 3, 20));
    ilG_floatspace_addPos(&fs, lights, lightp);

    ilG_light lightl;
    lightl.color = il_vec3_new(.8*2, .7*2, .2*2);
    lightl.radius = 50;
    ilG_handle_addLight(lights, lightl);

    il_storage_void sv = {&fs, NULL};
    ilE_register(&context->tick, ILE_DONTCARE, ILE_ANY, update_camera, il_value_opaque(sv));

    context->initialTitle = strdup("Teapots Demo");
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
