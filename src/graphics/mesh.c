#include "mesh.h"

#include <stdlib.h>
//#include <stdio.h>
#include <GL/glew.h>

//#include "graphics/obj.h"
#include "graphics/drawable3d.h"
#include "graphics/context.h"
#include "graphics/glutil.h"
#include "graphics/arrayattrib.h"
#include "graphics/bindable.h"
#include "asset/mesh.h"
#include "asset/path.h"
#include "asset/node.h"

struct ilG_mesh {
    ilG_drawable3d drawable;
    GLuint vbo;
    GLuint vao;
    GLint count;
    GLenum type;
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
    glDrawArrays(mesh->type, 0, mesh->count);
    ilG_testError("Failed to draw mesh");
}

static ilG_bindable mesh_bindable = {
    .name = "il.graphics.bindable",
    .bind = bind,
    .action = draw,
    .unbind = NULL
};

ilG_drawable3d* ilG_mesh(ilA_mesh* self)
{
    static GLenum mapping[] = {
        GL_POINTS,                  //ILA_MESH_POINTS
        GL_LINES,                   //ILA_MESH_LINES
        GL_LINE_STRIP,              //ILA_MESH_LINE_STRIP
        GL_LINE_LOOP,               //ILA_MESH_LINE_LOOP
        GL_LINE_STRIP_ADJACENCY,    //ILA_MESH_LINE_STRIP_ADJACENCY
        GL_LINES_ADJACENCY,         //ILA_MESH_LINES_ADJACENCY
        GL_TRIANGLES,               //ILA_MESH_TRIANGLES
        GL_TRIANGLE_STRIP,          //ILA_MESH_TRIANGLE_STRIP
        GL_TRIANGLE_FAN,            //ILA_MESH_TRIANGLE_FAN
        GL_TRIANGLE_STRIP_ADJACENCY,//ILA_MESH_TRIANGLE_STRIP_ADJACENCY
        GL_TRIANGLES_ADJACENCY,     //ILA_MESH_TRIANGLES_ADJACENCY
        GL_PATCHES,                 //ILA_MESH_PATCHES
    };
    struct ilG_mesh* m = il_new(&ilG_mesh_type);
    m->type = mapping[self->mode];
    m->count = self->num_vertices;
    ilG_testError("Unknown");
    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);
    //m->vbo = ilG_obj_to_gl(mesh, &m->count);
    glGenBuffers(1, &m->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    size_t per_vertex = 
        (sizeof(float) * 4 * !!self->position) +
        (sizeof(float) * 4 * !!self->texcoord) +
        (sizeof(float) * 4 * !!self->normal) +
        (sizeof(unsigned char) * 4 * !!self->ambient) +
        (sizeof(unsigned char) * 4 * !!self->diffuse) +
        (sizeof(unsigned char) * 4 * !!self->specular);
    glBufferData(GL_ARRAY_BUFFER, per_vertex * self->num_vertices, NULL, GL_STATIC_DRAW);
    size_t offset = 0, size;
    if (self->position) {
        ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_POSITION);
        size = self->num_vertices * sizeof(float) * 4;
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, self->position);
        glVertexAttribPointer(ILG_ARRATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid*)offset);
        glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
        offset += size;
    }
    if (self->texcoord) {
        ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_TEXCOORD);
        size = self->num_vertices * sizeof(float) * 4;
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, self->texcoord);
        glVertexAttribPointer(ILG_ARRATTR_TEXCOORD, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid*)offset);
        glEnableVertexAttribArray(ILG_ARRATTR_TEXCOORD);
        offset += size;
    }
    if (self->normal) {
        ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_NORMAL);
        size = self->num_vertices * sizeof(float) * 4;
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, self->normal);
        glVertexAttribPointer(ILG_ARRATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid*)offset);
        glEnableVertexAttribArray(ILG_ARRATTR_NORMAL);
        offset += size;
    }
    if (self->ambient) {
        ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_AMBIENT);
        size = self->num_vertices * sizeof(unsigned char) * 4;
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, self->ambient);
        glVertexAttribPointer(ILG_ARRATTR_AMBIENT, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (GLvoid*)offset);
        glEnableVertexAttribArray(ILG_ARRATTR_AMBIENT);
        offset += size;
    }
    if (self->diffuse) {
        ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_DIFFUSE);
        size = self->num_vertices * sizeof(unsigned char) * 4;
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, self->diffuse);
        glVertexAttribPointer(ILG_ARRATTR_DIFFUSE, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (GLvoid*)offset);
        glEnableVertexAttribArray(ILG_ARRATTR_DIFFUSE);
        offset += size;
    }
    if (self->specular) {
        ILG_SETATTR(m->drawable.attrs, ILG_ARRATTR_SPECULAR);
        size = self->num_vertices * sizeof(unsigned char) * 4;
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, self->specular);
        glVertexAttribPointer(ILG_ARRATTR_SPECULAR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (GLvoid*)offset);
        glEnableVertexAttribArray(ILG_ARRATTR_SPECULAR);
        offset += size;
    }
    ilG_testError("Error uploading mesh");
    return &m->drawable;
}

ilA_mesh *ilA_mesh_parseObj(const char *filename, const char *data, size_t length);
ilG_drawable3d* ilG_mesh_fromfile(const char *name)
{
    size_t length;
    void *data;
    il_base *file = ilA_contents_chars(name, &length, &data, NULL);
    ilA_mesh *mesh = ilA_mesh_parseObj(name, data, length);
    il_unref(file);
    ilG_drawable3d *drawable = ilG_mesh(mesh);
    ilA_mesh_free(mesh);
    return drawable;
}


