#include "mesh.h"

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

