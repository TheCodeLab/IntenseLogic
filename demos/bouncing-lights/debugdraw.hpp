#include <btBulletDynamicsCommon.h>
#include <LinearMath/btIDebugDraw.h>
#include <vector>

#include "tgl/tgl.h"

extern "C" {
#include "graphics/context.h"
#include "graphics/material.h"
}

namespace BouncingLights {

struct Vertex {
    Vertex(const btVector3 &pos, const btVector3 &col) :
        pos(pos),
        col(col)
    {}
    btVector3 pos;
    btVector3 col;
};

class DebugDraw : public btIDebugDraw {
    ilG_matid mat;
    std::vector<Vertex> lines;
    GLuint vbo, vao;
    GLuint vp_loc;
    int debugMode;
    unsigned count;
    ilG_renderman *rm = nullptr;

    static void upload_cb(void*);
    static void view(void *ptr, ilG_rendid id, il_mat *mats);
    static void free(void *ptr);
    static bool build(void *ptr, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out);
public:
    DebugDraw();

    ilG_builder builder();

    void render();
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
