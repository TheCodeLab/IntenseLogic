#include "debugdraw.hpp"

#include <btBulletDynamicsCommon.h>
#include <LinearMath/btIDebugDraw.h>

using namespace std;
using namespace BouncingLights;

extern "C" {
#include "graphics/transform.h"
#include "math/matrix.h"
#include "graphics/renderer.h"
#include "graphics/arrayattrib.h"
#include "util/log.h"
}

DebugDraw::DebugDraw()
    : debugMode(DBG_DrawAabb),
      count(0)
{}

void DebugDraw::view(void *ptr, ilG_rendid id, il_mat *mats)
{
    (void)id;
    DebugDraw &self = *reinterpret_cast<DebugDraw*>(ptr);
    ilG_material *mat = ilG_renderman_findMaterial(self.rm, self.mat);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    ilG_material_bind(mat);
    ilG_material_bindMatrix(mat, self.vp_loc, mats[0]);
    glBindVertexArray(self.vao);
    glBindBuffer(GL_ARRAY_BUFFER, self.vbo);
    glDrawArrays(GL_LINES, 0, self.count);
}

void DebugDraw::free(void *ptr)
{
    DebugDraw &debugdraw = *reinterpret_cast<DebugDraw*>(ptr);
    ilG_renderman_delMaterial(debugdraw.rm, debugdraw.mat);
    delete &debugdraw;
}

bool DebugDraw::build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out)
{
    (void)id;
    DebugDraw &self = *reinterpret_cast<DebugDraw*>(ptr);

    self.rm = rm;

    ilG_material m;
    ilG_material_init(&m);
    ilG_material_name(&m, "Bullet Line Renderer");
    ilG_material_arrayAttrib(&m, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(&m, ILG_ARRATTR_AMBIENT, "in_Ambient");
    if (!ilG_renderman_addMaterialFromFile(rm, m, "bullet-debug.vert", "bullet-debug.frag", &self.mat, &out->error)) {
        return false;
    }
    self.vp_loc = ilG_material_getLoc(ilG_renderman_findMaterial(rm, self.mat), "vp");

    glGenBuffers(1, &self.vbo);
    glGenVertexArrays(1, &self.vao);
    glBindBuffer(GL_ARRAY_BUFFER, self.vbo);
    glBindVertexArray(self.vao);
    glVertexAttribPointer(ILG_ARRATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, pos));
    glVertexAttribPointer(ILG_ARRATTR_AMBIENT, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, col));
    glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
    glEnableVertexAttribArray(ILG_ARRATTR_AMBIENT);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);

    int *types = reinterpret_cast<int*>(calloc(1, sizeof(int)));
    types[0] = ILG_VP;
    out->free = &DebugDraw::free;
    out->view = &DebugDraw::view;
    out->types = types;
    out->num_types = 1;
    out->name = strdup("DebugDraw");
    return true;
}

ilG_builder DebugDraw::builder()
{
    return ilG_builder_wrap(this, &DebugDraw::build);
}

void DebugDraw::upload_cb(void *ptr)
{
    DebugDraw &self = *reinterpret_cast<DebugDraw*>(ptr);
    glBindBuffer(GL_ARRAY_BUFFER, self.vbo);
    glBufferData(GL_ARRAY_BUFFER, self.lines.size() * sizeof(Vertex), self.lines.data(), GL_DYNAMIC_DRAW);
    self.count = self.lines.size();
    self.lines.clear();
}

void DebugDraw::upload()
{
    ilG_renderman_upload(rm, upload_cb, this);
}

void DebugDraw::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{
    lines.push_back(Vertex(from, color));
    lines.push_back(Vertex(to, color));
}

void DebugDraw::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &fromColor, const btVector3 &toColor)
{
    lines.push_back(Vertex(from, fromColor));
    lines.push_back(Vertex(to, toColor));
}

void DebugDraw::reportErrorWarning(const char *str)
{
    il_warning("bullet: %s", str);
}

void DebugDraw::draw3dText(const btVector3 &location, const char *textString)
{
    (void)location; (void)textString;
}

void DebugDraw::setDebugMode(int mode)
{
    debugMode = mode;
}

int DebugDraw::getDebugMode() const
{
    return debugMode;
}

void DebugDraw::drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color)
{
    (void)PointOnB; (void)normalOnB; (void)distance; (void)lifeTime; (void)color;
}
