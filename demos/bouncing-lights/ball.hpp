#ifndef BALL_H
#define BALL_H

#include <GL/glew.h>
#include <unordered_map>

extern "C" {
#include "graphics/material.h"
#include "graphics/mesh.h"
#include "graphics/renderer.h"
#include "math/vector.h"
}

namespace BouncingLights {

class BallRenderer {
    ilG_mesh mesh;
    ilG_material mat;
    GLuint mvp_loc, imt_loc, col_loc;
    std::unordered_map<unsigned, il_vec3> cols;

    static void free(void *ptr);
    static void draw(void *obj, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats);
    static bool build(void *obj, ilG_rendid id, ilG_context *context, ilG_buildresult *out);
public:
    BallRenderer();
    ilG_builder builder();
    void add_col(unsigned obj, il_vec3 col)
    {
        cols.insert(std::make_pair(obj, col));
    }
};

}

#endif