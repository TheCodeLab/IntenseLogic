#include "ball.hpp"

extern "C" {
#include "graphics/context.h"
#include "graphics/fragdata.h"
#include "graphics/material.h"
#include "graphics/mesh.h"
#include "graphics/renderer.h"
#include "graphics/transform.h"
#include "math/matrix.h"
}

enum {
    MVP = 0,
    IMT = 1
};

using namespace BouncingLights;

extern ilA_fs demo_fs;

void BallRenderer::free(void *ptr)
{
    BallRenderer &self = *reinterpret_cast<BallRenderer*>(ptr);
    ilG_mesh_free(&self.mesh);
    ilG_renderman_delMaterial(self.rm, self.mat);
}

void BallRenderer::draw(void *obj, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats)
{
    (void)id, (void)objects;
    BallRenderer &self = *reinterpret_cast<BallRenderer*>(obj);
    ilG_material *mat = ilG_renderman_findMaterial(self.rm, self.mat);
    ilG_mesh_bind(&self.mesh);
    ilG_material_bind(mat);
    for (unsigned i = 0; i < num_mats; i++) {
        ilG_material_bindMatrix(mat, self.mvp_loc, mats[MVP][i]);
        ilG_material_bindMatrix(mat, self.imt_loc, mats[IMT][i]);
        il_vec3 c = self.cols[objects[i]];
        glUniform3f(self.col_loc, c.x, c.y, c.z);
        ilG_mesh_draw(&self.mesh);
    }
}

bool BallRenderer::build(void *obj, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    BallRenderer &b = *reinterpret_cast<BallRenderer*>(obj);

    b.rm = rm;

    ilG_material m;
    ilG_material_init(&m);
    ilG_material_name(&m, "Ball Material");
    ilG_material_fragData(&m, ILG_FRAGDATA_NORMAL, "out_Normal");
    ilG_material_fragData(&m, ILG_FRAGDATA_ACCUMULATION, "out_Ambient");
    ilG_material_fragData(&m, ILG_FRAGDATA_DIFFUSE, "out_Diffuse");
    ilG_material_fragData(&m, ILG_FRAGDATA_SPECULAR, "out_Specular");
    ilG_material_arrayAttrib(&m, ILG_MESH_POS, "in_Position");
    if (!ilG_material_vertex_file(&m, "glow.vert", &out->error)) {
        return false;
    }
    if (!ilG_material_fragment_file(&m, "glow.frag", &out->error)) {
        return false;
    }
    if (!ilG_material_link(&m, &out->error)) {
        return false;
    }
    b.mvp_loc = ilG_material_getLoc(&m, "mvp");
    b.imt_loc = ilG_material_getLoc(&m, "imt");
    b.col_loc = ilG_material_getLoc(&m, "col");
    b.mat = ilG_renderman_addMaterial(b.rm, m);

    if (!ilG_mesh_fromfile(&b.mesh, &demo_fs, "demos/bouncing-lights/sphere.obj")) {
        return false;
    }
    if (!ilG_mesh_build(&b.mesh)) {
        return false;
    }

    int *types = (int*)calloc(2, sizeof(int));
    types[MVP] = ILG_MVP;
    types[IMT] = ILG_IMT;
    out->free = &BallRenderer::free;
    out->draw = &BallRenderer::draw;
    out->types = types;
    out->num_types = 2;
    out->obj = obj;
    out->name = strdup("Ball");
    return true;
}

namespace BouncingLights {

ilG_builder BallRenderer::builder()
{
    return ilG_builder_wrap(this, &BallRenderer::build);
}

}
