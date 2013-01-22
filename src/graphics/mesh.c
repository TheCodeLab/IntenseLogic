#include "mesh.h"

#include <stdlib.h>
#include <GL/glew.h>

#include "graphics/obj.h"
#include "graphics/drawable3d.h"
#include "graphics/context.h"
#include "graphics/glutil.h"
#include "graphics/arrayattrib.h"

struct ilG_mesh {
    ilG_drawable3d drawable;
    ilG_obj_mesh * mesh;
    GLuint vbo;
    GLuint vao;
    GLint count;
};

static void bind(ilG_context* context, void* ctx)
{
    (void)ctx;
    struct ilG_mesh* mesh = (struct ilG_mesh*)context->drawable;

    ilG_testError("Unknown error");
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    ilG_testError("Failed to bind mesh");
}

static void draw(ilG_context* context, struct il_positionable* pos, void * ctx)
{
    (void)ctx, (void)pos;
    struct ilG_mesh* mesh = (struct ilG_mesh*)context->drawable;

    ilG_testError("Unknown error");
    glDrawArrays(GL_TRIANGLES, 0, mesh->count);
    ilG_testError("Failed to draw mesh");
}

ilG_drawable3d* ilG_mesh_fromObj(ilG_obj_mesh * mesh)
{
    struct ilG_mesh* m = calloc(sizeof(struct ilG_mesh), 1);
    m->mesh = mesh;
    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);
    m->vbo = ilG_obj_to_gl(mesh, &m->count);
    if (mesh->name && mesh->group) {
        char *s = calloc(1, strlen(mesh->name) + strlen(mesh->group) + 2); // 1 for NUL, 1 for .
        sprintf(s, "%s.%s", mesh->group, mesh->name);
        m->drawable.name = s;
    } else if (mesh->name) {
        m->drawable.name = mesh->name;
    } else if (mesh->group) {
        m->drawable.name = mesh->group;
    } else {
        m->drawable.name = "Unnamed Mesh";
    }
    // TODO: actually check for mesh attributes
    ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_POSITION);
    ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_TEXCOORD);
    ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_NORMAL);
    m->drawable.bind = &bind;
    m->drawable.draw = &draw;
    ilG_drawable3d_assignId(&m->drawable);
    return &m->drawable;
}

ilG_drawable3d* ilG_mesh_fromFile(const char *name)
{
    ilG_obj_file f = ilG_obj_readfile(name);
    return ilG_mesh_fromObj(f.first_mesh);
}


