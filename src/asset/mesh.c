#include "mesh.h"

#include <stdlib.h>
#include <assert.h>

ilA_mesh *ilA_mesh_new(enum ilA_mesh_attrib attribs, size_t vertices)
{
    ilA_mesh *self = calloc(1, sizeof(ilA_mesh));

    self->num_vertices = vertices;
    if (attribs & ILA_MESH_POSITION) {
        self->position = calloc(vertices, sizeof(float) * 4);
    }
    if (attribs & ILA_MESH_TEXCOORD) {
        self->texcoord = calloc(vertices, sizeof(float) * 4);
    }
    if (attribs & ILA_MESH_NORMAL) {
        self->normal = calloc(vertices, sizeof(float) * 4);
    }
    if (attribs & ILA_MESH_AMBIENT) {
        self->ambient = calloc(vertices, sizeof(unsigned char) * 4);
    }
    if (attribs & ILA_MESH_DIFFUSE) {
        self->diffuse = calloc(vertices, sizeof(unsigned char) * 4);
    }
    if (attribs & ILA_MESH_SPECULAR) {
        self->specular = calloc(vertices, sizeof(unsigned char) * 4);
    }

    return self;
}

ilA_mesh *ilA_mesh_copy(const ilA_mesh *mesh)
{
    ilA_mesh *self = calloc(1, sizeof(ilA_mesh));
    self->mode = mesh->mode;
    self->num_vertices = mesh->num_vertices;
    self->texcoord_size = mesh->texcoord_size;
    size_t nv = self->num_vertices;

    if (mesh->position) {
        self->position = calloc(nv, sizeof(float[4]));
        memcpy(self->position, mesh->position, sizeof(float[4]) * nv);
    }
    if (mesh->texcoord) {
        self->texcoord = calloc(nv, sizeof(float[4]));
        memcpy(self->texcoord, mesh->texcoord, sizeof(float[4]) * nv);
    }
    if (mesh->normal) {
        self->normal = calloc(nv, sizeof(float[4]));
        memcpy(self->normal, mesh->normal, sizeof(float[4]) * nv);
    }
    if (mesh->ambient) {
        self->ambient = calloc(nv, sizeof(unsigned char[4]));
        memcpy(self->ambient, mesh->ambient, sizeof(unsigned char[4]) * nv);
    }
    if (mesh->diffuse) {
        self->diffuse = calloc(nv, sizeof(unsigned char[4]));
        memcpy(self->diffuse, mesh->diffuse, sizeof(unsigned char[4]) * nv);
    }
    if (mesh->specular) {
        self->specular = calloc(nv, sizeof(unsigned char[4]));
        memcpy(self->specular, mesh->specular, sizeof(unsigned char[4]) * nv);
    }

    return self;
}

ilA_mesh *ilA_mesh_loadfile(ilA_fs *fs, const char *path)
{
    ilA_map map;
    if (!ilA_mapfile(fs, &map, ILA_READ, path, -1)) {
        ilA_printerror(&map.err);
        return NULL;
    }
    ilA_mesh *mesh = ilA_mesh_loadmem(fs, path, map.data, map.size);
    ilA_unmapfile(&map);
    return mesh;
}

ilA_mesh *ilA_mesh_parseObj(ilA_fs *fs, const char *filename, const char *data, size_t length);
ilA_mesh *ilA_mesh_loadmem(ilA_fs *fs, const char *filename, const void *data, size_t length)
{
    return ilA_mesh_parseObj(fs, filename, data, length); // TODO: Format detection
}


void ilA_mesh_free(ilA_mesh *self)
{
    if (self->position) {
        free(self->position);
    }
    if (self->texcoord) {
        free(self->texcoord);
    }
    if (self->normal) {
        free(self->normal);
    }
    if (self->ambient) {
        free(self->ambient);
    }
    if (self->diffuse) {
        free(self->diffuse);
    }
    if (self->specular) {
        free(self->specular);
    }
    free(self);
}

void ilA_debugLines(size_t num, float f, float *positions, float *normals, float *out_verts)
{
    assert(positions && normals && out_verts);
    for (unsigned i = 0; i < num; i++) {
        const unsigned ps = 3, p = i*3;
        const unsigned vs = ps*2, v1 = i*vs, v2 = v1 + ps;

        memcpy(out_verts + v1, positions + p, sizeof(float) * vs);
        out_verts[v2 + 0] = positions[p + 0] + normals[p + 0] * f;
        out_verts[v2 + 1] = positions[p + 1] + normals[p + 0] * f;
        out_verts[v2 + 2] = positions[p + 2] + normals[p + 0] * f;
    }
}
