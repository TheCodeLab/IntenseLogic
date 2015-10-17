#include "renderer.h"

#include <stdbool.h>
#include <limits.h>

#include "graphics/transform.h"
#include "graphics/material.h"
#include "util/array.h"
#include "util/log.h"

void ilG_renderman_free(ilG_renderman *rm)
{
#define foreach(list) for (unsigned i = 0; i < list.length; i++)
    tgl_fbo_free(&rm->gbuffer);
    tgl_fbo_free(&rm->accum);
}

void ilG_renderman_init(ilG_renderman *rm)
{
    memset(rm, 0, sizeof(*rm));
    tgl_fbo_init(&rm->gbuffer);
    tgl_fbo_init(&rm->accum);
}

void ilG_renderman_setup(ilG_renderman *rm, bool msaa, bool hdr)
{
    GLenum type = msaa? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_RECTANGLE;
    GLenum afmt = hdr? GL_RGBA16F : GL_RGBA8;
    tgl_fbo_numTargets(&rm->gbuffer, ILG_GBUFFER_NUMATTACHMENTS);
    // Per GL 3.1 spec §4.2 (page 182, actual 195):
    // Each COLOR_ATTACHMENTi adheres to COLOR_ATTACHMENTi = COLOR_ATTACHMENT0 + i.
    tgl_fbo_texture(&rm->gbuffer, ILG_GBUFFER_ALBEDO, type, GL_RGB8, GL_RGB,
                    GL_COLOR_ATTACHMENT0 + ILG_GBUFFER_ALBEDO, GL_UNSIGNED_BYTE);
    tgl_fbo_texture(&rm->gbuffer, ILG_GBUFFER_NORMAL, type, GL_RGB8_SNORM, GL_RGB,
                    GL_COLOR_ATTACHMENT0 + ILG_GBUFFER_NORMAL, GL_UNSIGNED_BYTE);
    tgl_fbo_texture(&rm->gbuffer, ILG_GBUFFER_REFRACTION, type, GL_R8, GL_RED,
                    GL_COLOR_ATTACHMENT0 + ILG_GBUFFER_REFRACTION, GL_UNSIGNED_BYTE);
    tgl_fbo_texture(&rm->gbuffer, ILG_GBUFFER_GLOSS, type, GL_R16F, GL_RED,
                    GL_COLOR_ATTACHMENT0 + ILG_GBUFFER_GLOSS, GL_FLOAT);
    tgl_fbo_texture(&rm->gbuffer, ILG_GBUFFER_EMISSION, type, GL_R16F, GL_RED,
                    GL_COLOR_ATTACHMENT0 + ILG_GBUFFER_EMISSION, GL_FLOAT);
    tgl_fbo_texture(&rm->gbuffer, ILG_GBUFFER_DEPTH, type, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT, GL_FLOAT);
    tgl_fbo_numTargets(&rm->accum, 1);
    tgl_fbo_texture(&rm->accum, 0, type, afmt, GL_RGBA, GL_COLOR_ATTACHMENT0, hdr? GL_FLOAT : GL_UNSIGNED_BYTE);
    if (msaa) {
        tgl_fbo_multisample(&rm->gbuffer, ILG_GBUFFER_DEPTH, msaa, false);
        tgl_fbo_multisample(&rm->gbuffer, ILG_GBUFFER_NORMAL, msaa, false);
        tgl_fbo_multisample(&rm->gbuffer, ILG_GBUFFER_ALBEDO, msaa, false);
        tgl_fbo_multisample(&rm->gbuffer, ILG_GBUFFER_REFRACTION, msaa, false);
        tgl_fbo_multisample(&rm->gbuffer, ILG_GBUFFER_GLOSS, msaa, false);
        tgl_fbo_multisample(&rm->gbuffer, ILG_GBUFFER_EMISSION, msaa, false);
        tgl_fbo_multisample(&rm->accum, 0, msaa, false);
    }
    tgl_check("Unable to generate framebuffer");
}

bool ilG_renderman_resize(ilG_renderman *self, int w, int h)
{
    self->width = w;
    self->height = h;

    if (!tgl_fbo_build(&self->gbuffer, w, h) || !tgl_fbo_build(&self->accum, w, h)) {
        return false;
    }
    tgl_check("Error setting up screen");

    return true;
}

#define log(...) do { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } while (0)
void ilG_material_print(ilG_material *mat)
{
    log("name: \"%s\"; vertex shader ID: %u; fragment shader ID: %u",
        mat->name, mat->vert, mat->frag);
}

void ilG_renderman_print(ilG_renderman *rm)
{
    log("Materials:");
    for (unsigned i = 0; i < rm->materials.length; i++) {
        fprintf(stderr, "  %u: ", i);
        ilG_material_print(&rm->materials.data[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Access and modification

ilG_material *ilG_renderman_findMaterial(ilG_renderman *self, ilG_matid mat)
{
    return &self->materials.data[mat.id];
}

ilG_shader *ilG_renderman_findShader(ilG_renderman *self, unsigned id)
{
    return &self->shaders.data[id];
}

ilG_matid ilG_renderman_addMaterial(ilG_renderman *self, ilG_material mat)
{
    unsigned id = self->materials.length;
    IL_APPEND(self->materials, mat);
    ilG_matid matid = (ilG_matid){id};
    if (self->material_creation) {
        self->material_creation(matid, self->material_creation_data);
    }
    return matid;
}

__attribute__((warn_unused_result))
bool ilG_renderman_addMaterialFromShader(ilG_renderman *self, ilG_material mat, ilG_shader vert,
                                         ilG_shader frag, ilG_matid *out, char **error)
{
    mat.vert = ilG_renderman_addShader(self, vert);
    mat.frag = ilG_renderman_addShader(self, frag);
    if (!ilG_material_link(&mat, &vert, &frag, error)) {
        return false;
    }
    *out = ilG_renderman_addMaterial(self, mat);
    return true;
}

__attribute__((warn_unused_result))
bool ilG_renderman_addMaterialFromFile(ilG_renderman *self, ilG_material mat, const char *vertpath,
                                       const char *fragpath, ilG_matid *out, char **error)
{
    ilG_shader vert, frag;
    if (!ilG_shader_file(&vert, vertpath, GL_VERTEX_SHADER, error) ||
        !ilG_shader_file(&frag, fragpath, GL_FRAGMENT_SHADER, error) ||
        !ilG_shader_compile(&vert, error) ||
        !ilG_shader_compile(&frag, error)) {
        return false;
    }
    return ilG_renderman_addMaterialFromShader(self, mat, vert, frag, out, error);
}

unsigned ilG_renderman_addShader(ilG_renderman *self, ilG_shader shader)
{
    IL_APPEND(self->shaders, shader);
    return self->shaders.length - 1;
}

bool ilG_renderman_delMaterial(ilG_renderman *self, ilG_matid mat)
{
    (void)self, (void)mat;
    return false; // TODO: Material deletion
}
