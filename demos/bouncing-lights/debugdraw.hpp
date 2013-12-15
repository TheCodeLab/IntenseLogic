#include <btBulletDynamicsCommon.h>
#include <LinearMath/btIDebugDraw.h>
#include <vector>
#include <GL/glew.h>

extern "C" {
#include "graphics/context.h"
#include "graphics/material.h"
}

namespace il {
namespace bouncinglights {

struct Vertex {
    Vertex(const btVector3 &pos, const btVector3 &col) :
        pos(pos),
        col(col)
    {}
    btVector3 pos;
    btVector3 col;
};

class DebugDraw : public btIDebugDraw {
    ilG_material *mat;
    std::vector<Vertex> lines;
    GLuint vbo, vao;
    int debugMode;
    unsigned count;
    ilG_context *context;
public:
    DebugDraw(ilG_context *ctx);
    void render();
    void compile();
    void upload();
    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color);
    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &fromColor, const btVector3 &toColor);
    void reportErrorWarning(const char *str);
    void draw3dText(const btVector3 &location, const char *textString);
    void setDebugMode(int debugMode);
    int getDebugMode() const;
    void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color);
};

}
}

