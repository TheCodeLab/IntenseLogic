#include "ball.hpp"

extern "C" {
#include "graphics/fragdata.h"
#include "graphics/material.h"
#include "graphics/mesh.h"
#include "graphics/renderer.h"
#include "math/matrix.h"
}

enum {
    MVP = 0,
    IMT = 1
};

using namespace BouncingLights;

void BallRenderer::free(void *ptr)
{
    BallRenderer &self = *reinterpret_cast<BallRenderer*>(ptr);
    ilG_mesh_free(&self.mesh);
    ilG_material_free(&self.mat);
}

void BallRenderer::draw(void *obj, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats)
{
    (void)id, (void)objects;
    BallRenderer &self = *reinterpret_cast<BallRenderer*>(obj);
    ilG_mesh_bind(&self.mesh);
    ilG_material_bind(&self.mat);
    for (unsigned i = 0; i < num_mats; i++) {
        ilG_material_bindMatrix(&self.mat, self.mvp_loc, mats[MVP][i]);
        ilG_material_bindMatrix(&self.mat, self.imt_loc, mats[IMT][i]);
        il_vec3 c = self.cols[objects[i]];
        glUniform3f(self.col_loc, c.x, c.y, c.z);
        ilG_mesh_draw(&self.mesh);
    }
}

bool BallRenderer::build(void *obj, ilG_rendid id, ilG_context *context, ilG_buildresult *out)
{
    (void)id;
    BallRenderer &b = *reinterpret_cast<BallRenderer*>(obj);
    if (ilG_material_link(&b.mat, context)) {
        return false;
    }
    b.mvp_loc = ilG_material_getLoc(&b.mat, "mvp");
    b.imt_loc = ilG_material_getLoc(&b.mat, "imt");
    b.col_loc = ilG_material_getLoc(&b.mat, "col");

    if (!ilG_mesh_build(&b.mesh, context)) {
        return false;
    }

    memset(out, 0, sizeof(ilG_buildresult));
    int *types = (int*)calloc(2, sizeof(int));
    types[MVP] = ILG_MVP;
    types[IMT] = ILG_IMT;
    out->free = &BallRenderer::free;
    out->draw = &BallRenderer::draw;
    out->types = types;
    out->num_types = 2;
    out->obj = obj;
    return true;
}

namespace BouncingLights {

BallRenderer::BallRenderer()
{
    ilG_material *m = &mat;
    ilG_material_init(m);
    ilG_material_vertex_file(m, "glow.vert");
    ilG_material_fragment_file(m, "glow.frag");
    ilG_material_name(m, "Ball Material");
    ilG_material_fragData(m, ILG_FRAGDATA_NORMAL, "out_Normal");
    ilG_material_fragData(m, ILG_FRAGDATA_ACCUMULATION, "out_Ambient");
    ilG_material_fragData(m, ILG_FRAGDATA_DIFFUSE, "out_Diffuse");
    ilG_material_fragData(m, ILG_FRAGDATA_SPECULAR, "out_Specular");
    ilG_material_arrayAttrib(m, ILG_MESH_POS, "in_Position");
    /*ilG_material_arrayAttrib(m, ILG_MESH_TEX, "in_Texcoord");
    ilG_material_arrayAttrib(m, ILG_MESH_NORM, "in_Normal");
    ilG_material_arrayAttrib(m, ILG_MESH_DIFFUSE, "in_Diffuse");
    ilG_material_arrayAttrib(m, ILG_MESH_SPECULAR, "in_Specular");*/

    ilG_mesh_fromfile(&mesh, "demos/bouncing-lights/sphere.obj");
}

ilG_builder BallRenderer::builder()
{
    return ilG_builder_wrap(this, &BallRenderer::build);
}

}
