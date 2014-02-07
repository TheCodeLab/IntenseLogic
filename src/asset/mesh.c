#include "mesh.h"

#include <stdlib.h>

#include "util/ilassert.h"
#include "util/alloc.h"

static il_allocator *aligned_16;

ilA_mesh *ilA_mesh_new(enum ilA_mesh_attrib attribs, size_t vertices)
{
    ilA_mesh *self = calloc(1, sizeof(ilA_mesh));

    if (!aligned_16) {
        aligned_16 = il_allocator_aligned(&il_default_alloc, 16);
    }
    self->num_vertices = vertices;
    if (attribs & ILA_MESH_POSITION) {
        self->position = il_alloc(aligned_16, sizeof(float) * 4 * vertices);
    }
    if (attribs & ILA_MESH_TEXCOORD) {
        self->texcoord = il_alloc(aligned_16, sizeof(float) * 4 * vertices);
    }
    if (attribs & ILA_MESH_NORMAL) {
        self->normal = il_alloc(aligned_16, sizeof(float) * 4 * vertices);
    }
    if (attribs & ILA_MESH_AMBIENT) {
        self->ambient = il_alloc(aligned_16, sizeof(unsigned char) * 4 * vertices);
    }
    if (attribs & ILA_MESH_DIFFUSE) {
        self->diffuse = il_alloc(aligned_16, sizeof(unsigned char) * 4 * vertices);
    }
    if (attribs & ILA_MESH_SPECULAR) {
        self->specular = il_alloc(aligned_16, sizeof(unsigned char) * 4 * vertices);
    }

    return self;
}

ilA_mesh *ilA_mesh_copy(const ilA_mesh *mesh)
{
    ilA_mesh *self = calloc(1, sizeof(ilA_mesh));
    self->mode = mesh->mode;
    self->num_vertices = mesh->num_vertices;
    self->texcoord_size = mesh->texcoord_size;

    if (mesh->position) {
        self->position = il_alloc(aligned_16, sizeof(float[4]) * self->num_vertices);
        memcpy(self->position, mesh->position, sizeof(float[4]) * self->num_vertices);
    }
    if (mesh->texcoord) {
        self->texcoord = il_alloc(aligned_16, sizeof(float[4]) * self->num_vertices);
        memcpy(self->texcoord, mesh->texcoord, sizeof(float[4]) * self->num_vertices);
    }
    if (mesh->normal) {
        self->normal = il_alloc(aligned_16, sizeof(float[4]) * self->num_vertices);
        memcpy(self->normal, mesh->normal, sizeof(float[4]) * self->num_vertices);
    }
    if (mesh->ambient) {
        self->ambient = il_alloc(aligned_16, sizeof(unsigned char[4]) * self->num_vertices);
        memcpy(self->ambient, mesh->ambient, sizeof(unsigned char[4]) * self->num_vertices);
    }
    if (mesh->diffuse) {
        self->diffuse = il_alloc(aligned_16, sizeof(unsigned char[4]) * self->num_vertices);
        memcpy(self->diffuse, mesh->diffuse, sizeof(unsigned char[4]) * self->num_vertices);
    }
    if (mesh->specular) {
        self->specular = il_alloc(aligned_16, sizeof(unsigned char[4]) * self->num_vertices);
        memcpy(self->specular, mesh->specular, sizeof(unsigned char[4]) * self->num_vertices);
    }

    return self;
}

ilA_mesh *ilA_mesh_load(il_base *file, const ilA_file *iface)
{
    size_t size;
    void *data = ilA_contents(iface, file, &size);
    return ilA_mesh_loadmem("", data, size); // TODO: add a file name getter to the file typeclass
}

ilA_mesh *ilA_mesh_loadfile(const char *path)
{
    size_t size;
    void *data;
    il_base *base = ilA_contents_chars(path, &size, &data, NULL);
    ilA_mesh *mesh = ilA_mesh_loadmem(path, data, size);
    il_unref(base);
    return mesh;
}

ilA_mesh *ilA_mesh_parseObj(const char *filename, const char *data, size_t length);
ilA_mesh *ilA_mesh_loadmem(const char *filename, const void *data, size_t length)
{
    return ilA_mesh_parseObj(filename, data, length); // TODO: Format detection
}


void ilA_mesh_free(ilA_mesh *self)
{
    if (self->position) {
        il_free(aligned_16, self->position);
    }
    if (self->texcoord) {
        il_free(aligned_16, self->texcoord);
    }
    if (self->normal) {
        il_free(aligned_16, self->normal);
    }
    if (self->ambient) {
        il_free(aligned_16, self->ambient);
    }
    if (self->diffuse) {
        il_free(aligned_16, self->diffuse);
    }
    if (self->specular) {
        il_free(aligned_16, self->specular);
    }
    free(self);
}

ilA_mesh *ilA_mesh_debugLines(ilA_mesh *self, float f)
{
    il_return_null_on_fail(self && self->position && self->normal);
    ilA_mesh *mesh = calloc(1, sizeof(ilA_mesh));
    mesh->mode = ILA_MESH_LINES;
    mesh->num_vertices = self->num_vertices * 2;
    mesh->position = calloc(sizeof(float) * 4, mesh->num_vertices);
    unsigned i;
    for (i = 0; i < self->num_vertices; i++) {
        memcpy(mesh->position[i * 2], self->position[i], sizeof(float) * 4);
        mesh->position[i*2 + 1][0] = self->position[i][0] + self->normal[i][0] * f;
        mesh->position[i*2 + 1][1] = self->position[i][1] + self->normal[i][1] * f;
        mesh->position[i*2 + 1][2] = self->position[i][2] + self->normal[i][2] * f;
    }
    return mesh;
}

