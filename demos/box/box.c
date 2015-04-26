#include <SDL.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include "graphics/context.h"
#include "graphics/floatspace.h"
#include "graphics/material.h"
#include "graphics/renderer.h"
#include "graphics/transform.h"
#include "math/matrix.h"
#include "util/log.h"

#ifndef M_PI
#define M_PI 3.1415926535
#endif

static const float cube[] = {
    // front
    -1.0, -1.0,  1.0,
    1.0, -1.0,  1.0,
    1.0,  1.0,  1.0,
    1.0,  1.0,  1.0,
    -1.0,  1.0,  1.0,
    -1.0, -1.0,  1.0,
    // top
    -1.0,  1.0,  1.0,
    1.0,  1.0,  1.0,
    1.0,  1.0, -1.0,
    1.0,  1.0, -1.0,
    -1.0,  1.0, -1.0,
    -1.0,  1.0,  1.0,
    // back
    1.0, -1.0, -1.0,
    -1.0, -1.0, -1.0,
    -1.0,  1.0, -1.0,
    -1.0,  1.0, -1.0,
    1.0,  1.0, -1.0,
    1.0, -1.0, -1.0,
    // bottom
    -1.0, -1.0, -1.0,
    1.0, -1.0, -1.0,
    1.0, -1.0,  1.0,
    1.0, -1.0,  1.0,
    -1.0, -1.0,  1.0,
    -1.0, -1.0, -1.0,
    // left
    -1.0, -1.0, -1.0,
    -1.0, -1.0,  1.0,
    -1.0,  1.0,  1.0,
    -1.0,  1.0,  1.0,
    -1.0,  1.0, -1.0,
    -1.0, -1.0, -1.0,
    // right
    1.0, -1.0,  1.0,
    1.0, -1.0, -1.0,
    1.0,  1.0, -1.0,
    1.0,  1.0, -1.0,
    1.0,  1.0,  1.0,
    1.0, -1.0,  1.0,
};

typedef struct box {
    ilG_renderman *rm;
    ilG_matid mat;
    GLuint vbo, vao;
    GLuint pos_loc;
    GLuint mvp_loc;
} box;

static void box_draw(void *obj, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats)
{
    (void)id, (void)objects;
    box *b = obj;
    ilG_material *mat = ilG_renderman_findMaterial(b->rm, b->mat);
    ilG_material_bind(mat);
    glBindVertexArray(b->vao);
    glBindBuffer(GL_ARRAY_BUFFER, b->vbo);
    for (unsigned i = 0; i < num_mats; i++) {
        ilG_material_bindMatrix(mat, b->mvp_loc, mats[0][i]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

static void box_free(void *obj)
{
    box *b = obj;
    ilG_renderman_delMaterial(b->rm, b->mat);
    glDeleteBuffers(1, &b->vao);
    glDeleteBuffers(1, &b->vbo);
    free(b);
}

static bool box_build(void *obj, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    box *b = obj;
    b->rm = rm;

    ilG_material m;
    ilG_material_init(&m);
    ilG_material_name(&m, "Box Shader");
    ilG_material_arrayAttrib(&m, 0, "in_Position");
    ilG_material_fragData(&m, ILG_CONTEXT_ALBEDO, "out_Color");
    if (!ilG_renderman_addMaterialFromFile(rm, m, "box.vert", "box.frag", &b->mat, &out->error)) {
        return false;
    }
    ilG_material *mat = ilG_renderman_findMaterial(rm, b->mat);
    b->pos_loc = ilG_material_getLoc(mat, "in_Position");
    b->mvp_loc = ilG_material_getLoc(mat, "mvp");

    glGenBuffers(1, &b->vbo);
    glGenVertexArrays(1, &b->vao);
    glBindBuffer(GL_ARRAY_BUFFER, b->vbo);
    glBindVertexArray(b->vao);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    int *types = calloc(1, sizeof(int));
    types[0] = ILG_MVP;
    out->free = box_free;
    out->draw = box_draw;
    out->types = types;
    out->num_types = 1;
    out->obj = obj;
    out->name = strdup("Box");
    return true;
}

static ilG_builder box_builder()
{
    box *b = calloc(1, sizeof(struct box));
    return ilG_builder_wrap(b, box_build);
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
    v.x = sinf(delta * M_PI * 2) * 5;
    v.y = 0;
    v.z = cosf(delta * M_PI * 2) * 5;
    il_quat q = il_quat_fromAxisAngle(0, 1, 0, delta * M_PI * 2);
    il_pos_setPosition(&fs->camera, v);
    il_pos_setRotation(&fs->camera, q);
}

void demo_start()
{
    ilG_context *context = ilG_context_new();
    ilG_context_hint(context, ILG_CONTEXT_DEBUG_RENDER, 1);
    ilG_handle geom, ambient, out, box;
    il_vec3 ambient_col = il_vec3_new(0.75, 0.64, 0.95);
    geom = ilG_build(ilG_geometry_builder(context), &context->manager);
    ambient = ilG_build(ilG_ambient_builder(context, &ambient_col), &context->manager);
    out = ilG_build(ilG_out_builder(context, NULL, NULL), &context->manager);
    box = ilG_build(box_builder(), &context->manager);

    ilG_handle_addRenderer(geom, box);
    ilG_handle_addRenderer(context->root, geom);
    ilG_handle_addRenderer(context->root, ambient);
    ilG_handle_addRenderer(context->root, out);

    ilG_floatspace fs;
    ilG_floatspace_init(&fs, 1);
    ilG_floatspace_build(&fs, context);
    fs.projection = il_mat_perspective(M_PI / 4.0, 4.0/3, .5, 200);

    il_pos_setPosition(&fs.camera, il_vec3_new(0, 0, 5));

    il_pos boxp = il_pos_new(&fs);
    ilG_floatspace_addPos(&fs, box, boxp);

    il_storage_void sv = {&fs, NULL};
    ilE_register(&context->tick, ILE_DONTCARE, ILE_ANY, update_camera, il_value_opaque(sv));

    context->initialTitle = strdup("Box Demo");
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
