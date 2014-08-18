#include "mesh.h"

#include <stdlib.h>
#include <GL/glew.h>

#include "asset/mesh.h"
#include "asset/node.h"
#include "graphics/arrayattrib.h"
#include "graphics/bindable.h"
#include "graphics/context.h"
#include "graphics/renderer.h"
#include "tgl/tgl.h"

int ilG_mesh_init(ilG_mesh *mesh, const ilA_mesh* self)
{
    memset(mesh, 0, sizeof(ilG_mesh));
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
    mesh->type = mapping[self->mode];
    mesh->count = self->num_vertices;
    mesh->mesh = ilA_mesh_copy(self);
    return 1;
}

ilA_mesh *ilA_mesh_parseObj(ilA_fs *fs, const char *filename, const char *data, size_t length);
int ilG_mesh_fromfile(ilG_mesh *mesh, ilA_fs *fs, const char *name)
{
    ilA_mesh *asset = ilA_mesh_loadfile(fs, name);
    int res = ilG_mesh_init(mesh, asset);
    ilA_mesh_free(asset);
    return res;
}

void ilG_mesh_free(ilG_mesh *mesh)
{
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteVertexArrays(1, &mesh->vao);
    if (mesh->mesh) {
        ilA_mesh_free(mesh->mesh);
    }
}

enum ilG_mesh_attribs ilG_mesh_build(ilG_mesh *m, ilG_context *context)
{
    (void)context;
    ilA_mesh *self = m->mesh;
    tgl_check("Unknown");
    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);
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
    unsigned attribs = 0;
    if (self->position) {
        attribs |= ILG_MESH_POS;
        size = self->num_vertices * sizeof(float) * 4;
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, self->position);
        glVertexAttribPointer(ILG_MESH_POS, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid*)offset);
        glEnableVertexAttribArray(ILG_MESH_POS);
        offset += size;
    }
    if (self->texcoord) {
        attribs |= ILG_MESH_TEX;
        size = self->num_vertices * sizeof(float) * 4;
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, self->texcoord);
        glVertexAttribPointer(ILG_MESH_TEX, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid*)offset);
        glEnableVertexAttribArray(ILG_MESH_TEX);
        offset += size;
    }
    if (self->normal) {
        attribs |= ILG_MESH_NORM;
        size = self->num_vertices * sizeof(float) * 4;
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, self->normal);
        glVertexAttribPointer(ILG_MESH_NORM, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid*)offset);
        glEnableVertexAttribArray(ILG_MESH_NORM);
        offset += size;
    }
    if (self->ambient) {
        attribs |= ILG_MESH_AMBIENT;
        size = self->num_vertices * sizeof(unsigned char) * 4;
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, self->ambient);
        glVertexAttribPointer(ILG_MESH_AMBIENT, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (GLvoid*)offset);
        glEnableVertexAttribArray(ILG_MESH_AMBIENT);
        offset += size;
    }
    if (self->diffuse) {
        attribs |= ILG_MESH_DIFFUSE;
        size = self->num_vertices * sizeof(unsigned char) * 4;
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, self->diffuse);
        glVertexAttribPointer(ILG_MESH_DIFFUSE, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (GLvoid*)offset);
        glEnableVertexAttribArray(ILG_MESH_DIFFUSE);
        offset += size;
    }
    if (self->specular) {
        attribs |= ILG_MESH_SPECULAR;
        size = self->num_vertices * sizeof(unsigned char) * 4;
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, self->specular);
        glVertexAttribPointer(ILG_MESH_SPECULAR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (GLvoid*)offset);
        glEnableVertexAttribArray(ILG_MESH_SPECULAR);
        offset += size;
    }
    ilA_mesh_free(self);
    m->mesh = NULL;
    tgl_check("Error uploading mesh");
    return attribs;
}

void ilG_mesh_bind(ilG_mesh *mesh)
{
    tgl_check("Unknown error");
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    tgl_check("Failed to bind mesh");
}

void ilG_mesh_draw(ilG_mesh *mesh)
{
    tgl_check("Unknown error");
    glDrawArrays(mesh->type, 0, mesh->count);
    tgl_check("Failed to draw mesh");
}
