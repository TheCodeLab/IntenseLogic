#ifndef BALL_H
#define BALL_H

#include <unordered_map>

#include "tgl/tgl.h"

extern "C" {
#include "graphics/material.h"
#include "graphics/mesh.h"
#include "graphics/renderer.h"
#include "math/vector.h"
}

namespace BouncingLights {

class BallRenderer {
    ilG_renderman *rm = nullptr;
    ilG_mesh mesh;
    ilG_matid mat;
    GLuint mvp_loc, imt_loc, col_loc;
    std::unordered_map<unsigned, il_vec3> cols;

    static void free(void *ptr);
    static void draw(void *obj, ilG_rendid id, il_mat **mats, const unsigned *objects, unsigned num_mats);
    static bool build(void *obj, ilG_rendid id, ilG_renderman *rm, ilG_buildresult *out);
public:
    ilG_builder builder();
    void add_col(unsigned obj, il_vec3 col)
    {
        cols.insert(std::make_pair(obj, col));
    }
};

}

#endif
