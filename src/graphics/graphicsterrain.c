#include "terrain.h"

#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <math.h>

#include "graphics/drawable3d.h"
#include "asset/asset.h"
#include "common/string.h"
#include "common/log.h"
#include "graphics/glutil.h"

enum terrain_type {
    NONE = 0,
    HEIGHTMAP,
    PHEIGHTMAP,
};

struct il_terrain {
    int type;
    int width, height;
    size_t size;
    void * data;
    int refs;
    void *destruct_ctx;
    void (*destruct)(il_terrain*, void*);
    void *point_ctx;
    double (*getPoint)(il_terrain*, void*, unsigned x, unsigned y,
                       double height);
    void *normal_ctx;
    sg_Vector3 (*getNormal)(il_terrain*, void*, unsigned x, unsigned y,
                            double z);
};

struct ilG_terrain {
    ilG_drawable3d drawable;
    il_terrain * terrain;
    GLuint *buf;
    GLuint program;
    void *draw_ctx;
    void (*draw)(ilG_terrain*, void*, const ilG_camera*,
                 const struct timeval*);
};

struct pheightmap {
    long long seed;
    float resolution;
    float viewdistance;
};

static void terrain_draw(const ilG_camera* cam,
                         struct ilG_drawable3d* drawable, const struct timeval* tv)
{
    ilG_terrain* ter = (ilG_terrain*)drawable;
    ter->draw(ter, ter->draw_ctx, cam, tv);
}

static void heightmap_draw(ilG_terrain* ter, void* ctx,
                           const ilG_camera* cam, const struct timeval* tv)
{
    (void)ctx, (void)cam, (void)tv;
    glUseProgram(ter->program);

    //activate the heightmap texture
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, ter->buf[0]);

    glDrawArrays(GL_TRIANGLE_FAN, 0, ter->terrain->width * ter->terrain->height);
}

static void pheightmap_draw(ilG_terrain* ter, void* ctx,
                            const ilG_camera* cam, const struct timeval* tv)
{
    (void)tv;
    struct pheightmap* pheightmap = ctx;

    glUseProgram(ter->program);
    ilG_testError("glUseProgram");

    ilG_bindUniforms(ter->program, cam, ter->drawable.positionable);
    ilG_testError("ilG_bindUniforms");

    GLint center = glGetUniformLocation(ter->program, "center");
    ilG_testError("glGetUniformLocation");
    glUniform2i(center,
                (int)floor(ter->drawable.positionable->position.x / pheightmap->resolution),
                (int)floor(ter->drawable.positionable->position.z / pheightmap->resolution)
               );
    ilG_testError("glUniform2i");

    glBindBuffer(GL_ARRAY_BUFFER, ter->buf[0]);
    ilG_testError("glBindBuffer");
    glBindVertexArray(ter->buf[2]);
    ilG_testError("glBindVertexArray");

    int width = pheightmap->viewdistance / pheightmap->resolution,
        size = width * width;

    glPrimitiveRestartIndex(-1);
    ilG_testError("glPrimitiveRestartIndex");

    glEnable(GL_PRIMITIVE_RESTART);
    ilG_testError("glEnable");

    glEnableVertexAttribArray(0);
    ilG_testError("glEnableVertexAttribArray");

    glDrawArrays(GL_TRIANGLE_STRIP, 0, size*2);
    ilG_testError("glDrawArrays");

    glDisableVertexAttribArray(0);
    ilG_testError("glDisableVertexAttribArray");

    glDisable(GL_PRIMITIVE_RESTART);
    ilG_testError("glDisable");
}

ilG_terrain* ilG_terrain_new(il_terrain* parent,
        il_positionable* positionable)
{
    ilG_terrain * ter = calloc(1, sizeof(ilG_terrain));
    ter->terrain = parent;
    ter->drawable.draw = (ilG_drawable3d_cb)&terrain_draw;
    switch(parent->type) {
    case HEIGHTMAP: {
        ter->buf = calloc(1, sizeof(GLuint));
        glGenTextures(1, ter->buf);
        glBindTexture(GL_TEXTURE_2D, ter->buf[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, parent->width, parent->height, 0,
                     GL_RED, GL_FLOAT, parent->data);

        GLuint  vertex = ilG_makeShader(GL_VERTEX_SHADER,
                                                IL_ASSET_READFILE("heightmap.vert")),
                         fragment = ilG_makeShader(GL_FRAGMENT_SHADER,
                                    IL_ASSET_READFILE("heightmap.frag")),
                                    program = glCreateProgram();

        glAttachShader(program, vertex);
        glAttachShader(program, fragment);

        ilG_linkProgram(program);

        ter->program = program;
        ter->draw = &heightmap_draw;
        break;
    }
    case PHEIGHTMAP: {
        struct pheightmap* pheightmap = parent->data;
        ter->buf = calloc(3, sizeof(GLuint));

        glGenVertexArrays(1, &ter->buf[2]);
        glBindVertexArray(ter->buf[2]);
        ilG_testError("Failed to create VAO");

        glGenBuffers(2, ter->buf);
        glBindBuffer(GL_ARRAY_BUFFER, ter->buf[0]);
        int width = pheightmap->viewdistance / pheightmap->resolution,
            size = width * width,
            x, y,
            *buf = calloc(size, sizeof(int)*2); // vector2 = 2 floats
        for (y = 0; y < width; y++) {
            for (x = 0; x < width; x++) {
                buf[y*width*2 + 2*x + 0] = x;
                buf[y*width*2 + 2*x + 1] = y;
            }
        }
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(int) * 2, buf,
                     GL_STATIC_DRAW);
        free(buf);
        ilG_testError("Failed to create VBO");

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ter->buf[1]);
        int rowsize = width*2 + 1; // width*2 indices, 1 end restart index
        buf = calloc((width-1) * rowsize, sizeof(int));
        // width-1 columns, because tiles are "in-between" vertices
        for (y = 0; y < width-1; y++) {
            for (x = 0; x < width-1; x++) {
                buf[y*rowsize + x*2 + 0] = y*width + x;
                buf[y*rowsize + x*2 + 1] = (y+1)*width + x;
            }
            buf[(y+1)*rowsize - 1] = -1; // restart index
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (width-1) * rowsize * sizeof(int),
                     buf, GL_STATIC_DRAW);
        free(buf);
        ilG_testError("Failed to create IBO");

        GLuint  vertex = ilG_makeShader(GL_VERTEX_SHADER,
                                                IL_ASSET_READFILE("pheightmap.vert")),
                         fragment = ilG_makeShader(GL_FRAGMENT_SHADER,
                                    IL_ASSET_READFILE("pheightmap.frag")),
                                    program = glCreateProgram();

        glAttachShader(program, vertex);
        glAttachShader(program, fragment);

        ilG_linkProgram(program);

        glBindBuffer(GL_ARRAY_BUFFER, ter->buf[0]);
        ilG_testError("glBindBuffer");
        GLint in_pos = glGetAttribLocation(program, "in_Position");
        ilG_testError("glGetAttribLocation");
        // index 0, 2 components, integer, non-normalized, tightly packed,
        // no offset
        glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 0, (GLvoid*)0);
        ilG_testError("glVertexAttribPointer");
        glEnableVertexAttribArray(in_pos);
        ilG_testError("Failed to bind vertex attributes");

        ter->program = program;
        ter->draw = &pheightmap_draw;
        ter->draw_ctx = pheightmap;
        break;
    }
    case NONE:
    default:
        // ???
        break;
    }

    ilG_drawable3d_setPositionable(&ter->drawable, positionable);

    return ter;
}

