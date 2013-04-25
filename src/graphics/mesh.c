#include "mesh.h"

#include <stdlib.h>
//#include <stdio.h>
#include <GL/glew.h>

#include "graphics/obj.h"
#include "graphics/drawable3d.h"
#include "graphics/context.h"
#include "graphics/glutil.h"
#include "graphics/arrayattrib.h"
#include "graphics/bindable.h"

struct ilG_mesh {
    ilG_drawable3d drawable;
    ilG_obj_mesh * mesh;
    GLuint vbo;
    GLuint vao;
    GLint count;
};

static ilG_bindable mesh_bindable;

static void mesh_init(void *self)
{
    struct ilG_mesh *mesh = self;
    (void)mesh;
    static int first_run = 1;
    if (first_run) {
        first_run = 0;
        il_impl(&ilG_mesh_type, &mesh_bindable);
    }
}

il_type ilG_mesh_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = mesh_init,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.mesh",
    .registry = NULL,
    .size = sizeof(struct ilG_mesh),
    .parent = &ilG_drawable3d_type
};

static void bind(void* obj)
{
    struct ilG_mesh* mesh = obj;

    ilG_testError("Unknown error");
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    ilG_testError("Failed to bind mesh");
}

static void draw(void* obj)
{
    struct ilG_mesh* mesh = obj;

    ilG_testError("Unknown error");
    glDrawArrays(GL_TRIANGLES, 0, mesh->count);
    ilG_testError("Failed to draw mesh");
}

static ilG_bindable mesh_bindable = {
    .name = "il.graphics.bindable",
    .hh = {0},
    .bind = bind,
    .action = draw,
    .unbind = NULL
};

ilG_drawable3d* ilG_mesh_fromObj(ilG_obj_mesh * mesh)
{
    struct ilG_mesh* m = il_new(&ilG_mesh_type);
    m->mesh = mesh;
    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);
    m->vbo = ilG_obj_to_gl(mesh, &m->count);
    /*if (mesh->name && mesh->group) {
        char *s = calloc(1, strlen(mesh->name) + strlen(mesh->group) + 2); // 1 for NUL, 1 for .
        sprintf(s, "%s.%s", mesh->group, mesh->name);
        m->drawable.name = s;
    } else if (mesh->name) {
        m->drawable.name = mesh->name;
    } else if (mesh->group) {
        m->drawable.name = mesh->group;
    } else {
        m->drawable.name = "Unnamed Mesh";
    }*/
    // TODO: actually check for mesh attributes
    ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_POSITION);
    ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_TEXCOORD);
    ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_NORMAL);
    ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_AMBIENT);
    ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_DIFFUSE);
    ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_SPECULAR);
    return &m->drawable;
}

ilG_drawable3d* ilG_mesh_fromFile(const char *name)
{
    ilG_obj_file f = ilG_obj_readfile(name);
    return ilG_mesh_fromObj(f.first_mesh);
}


