#include "mesh.h"

#include <stdlib.h>

#include "util/assert.h"

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

