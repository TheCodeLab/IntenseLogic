#include "debugdraw.hpp"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/LinearMath/btIDebugDraw.h>
#include <cstddef>

using namespace std;
using namespace il::bouncinglights;

extern "C" {
#include "common/world.h"
#include "common/world.h"
#include "graphics/camera.h"
#include "graphics/glutil.h"
#include "math/matrix.h"
#include "graphics/renderer.h"
#include "graphics/arrayattrib.h"
#include "graphics/fragdata.h"
#include "graphics/bindable.h"
#include "util/log.h"
}

void DebugDraw::constructor_cb(void *ptr)
{
    DebugDraw *self = (DebugDraw*)ptr;
    glGenBuffers(1, &self->vbo);
    glGenVertexArrays(1, &self->vao);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBindVertexArray(self->vao);
    glVertexAttribPointer(ILG_ARRATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, pos));
    glVertexAttribPointer(ILG_ARRATTR_AMBIENT, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, col));
    glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
    glEnableVertexAttribArray(ILG_ARRATTR_AMBIENT);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
}

DebugDraw::DebugDraw(ilG_context *ctx) : context(ctx)
{
    debugMode = DBG_DrawAabb;
    mat = (ilG_material*)ilG_material_new();
    ilG_material_name(mat, "Bullet Line Renderer");
    ilG_material_arrayAttrib(mat, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(mat, ILG_ARRATTR_AMBIENT, "in_Ambient");
    ilG_material_fragData(mat, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    ilG_material_matrix(mat, ILG_VP, "vp");
    ilG_context_upload(context, constructor_cb, this);
    compile();
    count = 0;
}

void DebugDraw::render()
{
    //printf("render: %u\n", count/2);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    ilG_bindable_unbind(context->drawableb, context->drawable);
    context->drawable = NULL;
    context->drawableb = NULL;
    ilG_bindable_swap(&context->materialb, (void**)&context->material, mat);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glDrawArrays(GL_LINES, 0, count);
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

void DebugDraw::compile()
{
    ilG_material_vertex_file(mat, "bullet-debug.vert");
    ilG_material_fragment_file(mat, "bullet-debug.frag");
    if (ilG_material_link(mat, context)) {

    }
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

