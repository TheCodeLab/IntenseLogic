#include "debugdraw.hpp"

#include <btBulletDynamicsCommon.h>
#include <LinearMath/btIDebugDraw.h>

using namespace std;
using namespace BouncingLights;

extern "C" {
#include "common/world.h"
#include "graphics/transform.h"
#include "math/matrix.h"
#include "graphics/renderer.h"
#include "graphics/arrayattrib.h"
#include "graphics/fragdata.h"
#include "graphics/bindable.h"
#include "util/log.h"
}

DebugDraw::DebugDraw()
{
    debugMode = DBG_DrawAabb;
    ilG_material_init(&this->mat);
    ilG_material *mat = &this->mat;
    ilG_material_vertex_file(mat, "bullet-debug.vert");
    ilG_material_fragment_file(mat, "bullet-debug.frag");
    ilG_material_name(mat, "Bullet Line Renderer");
    ilG_material_arrayAttrib(mat, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(mat, ILG_ARRATTR_AMBIENT, "in_Ambient");
    ilG_material_fragData(mat, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    count = 0;
}

void DebugDraw::view(void *ptr, ilG_rendid id, il_mat *mats)
{
    (void)id;
    DebugDraw *self = reinterpret_cast<DebugDraw*>(ptr);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    ilG_material_bind(&self->mat);
    ilG_material_bindMatrix(&self->mat, self->vp_loc, mats[0]);
    glBindVertexArray(self->vao);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glDrawArrays(GL_LINES, 0, self->count);
}

void DebugDraw::free(void *ptr)
{
    DebugDraw *debugdraw = reinterpret_cast<DebugDraw*>(ptr);
    delete debugdraw;
}

bool DebugDraw::build(void *ptr, ilG_rendid id, ilG_context *context, ilG_buildresult *out)
{
    (void)id;
    DebugDraw *self = reinterpret_cast<DebugDraw*>(ptr);

    self->context = context;

    if (ilG_material_link(&self->mat, context)) {
        return false;
    }
    self->vp_loc = ilG_material_getLoc(&self->mat, "vp");

    glGenBuffers(1, &self->vbo);
    glGenVertexArrays(1, &self->vao);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBindVertexArray(self->vao);
    glVertexAttribPointer(ILG_ARRATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, pos));
    glVertexAttribPointer(ILG_ARRATTR_AMBIENT, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, col));
    glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
    glEnableVertexAttribArray(ILG_ARRATTR_AMBIENT);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);

    memset(out, 0, sizeof(ilG_buildresult));
    int *types = reinterpret_cast<int*>(calloc(1, sizeof(int)));
    types[0] = ILG_VP;
    out->free = &DebugDraw::free;
    out->view = &DebugDraw::view;
    out->types = types;
    out->num_types = 1;
    return true;
}

ilG_builder DebugDraw::builder()
{
    return ilG_builder_wrap(this, &DebugDraw::build);
}

void DebugDraw::upload_cb(void *ptr)
{
    DebugDraw *self = (DebugDraw*)ptr;
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBufferData(GL_ARRAY_BUFFER, self->lines.size() * sizeof(Vertex), self->lines.data(), GL_DYNAMIC_DRAW);
    self->count = self->lines.size();
    self->lines.clear();
}

void DebugDraw::upload()
{
    ilG_context_upload(context, upload_cb, this);
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
