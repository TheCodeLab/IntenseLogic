#include "lightpass.h"

#include "graphics/stage.h"
#include "graphics/context.h"
#include "graphics/bindable.h"
#include "graphics/material.h"
#include "graphics/shape.h"
#include "graphics/fragdata.h"
#include "graphics/textureunit.h"
#include "graphics/arrayattrib.h"
#include "math/vector.h"
#include "math/matrix.h"

struct lightpass {
    ilG_stage stage;
    GLuint vao, vbo, ibo, lights_ubo, lights_index, mvp_ubo, mvp_index;
    GLint lights_size, mvp_size, lights_offset[3], mvp_offset[1];
    struct ilG_material* material;
    int invalidated;
};

il_type ilG_lightpass_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = NULL,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.lightpass",
    .registry = NULL,
    .size = sizeof(struct lightpass),
    .parent = &ilG_stage_type
};

static void size_uniform(ilG_material *self, GLint location, void *user)
{
    (void)self;
    ilG_context *context = user;
    glUniform2f(location, context->width, context->height);
}

static void draw_lights(ilG_stage *ptr)
{ 
    struct lightpass *self = (struct lightpass*)ptr;
    ilG_context *context = ptr->context;
    ilG_testError("Unknown");
    context->material = self->material;
    /*if (context->lightdata.invalidated) {
        if (!context->lightdata.created) {
            context->lightdata.lights_index = glGetUniformBlockIndex(context->material->program, "LightBlock");
            context->lightdata.mvp_index = glGetUniformBlockIndex(context->material->program, "MVPBlock");
            glGetActiveUniformBlockiv(context->material->program, context->lightdata.lights_index, GL_UNIFORM_BLOCK_DATA_SIZE, &context->lightdata.lights_size);
            glGetActiveUniformBlockiv(context->material->program, context->lightdata.mvp_index, GL_UNIFORM_BLOCK_DATA_SIZE, &context->lightdata.mvp_size);
            //GLubyte *lights_buf = malloc(lights_size), *mvp_buf = malloc(mvp_size);
            const GLchar *lights_names[] = {
                "position",
                "color",
                "radius"
            }, *mvp_names[] = {
                "mvp"
            };
            GLuint lights_indices[3], mvp_indices[1];
            glGetUniformIndices(context->material->program, 3, lights_names, lights_indices);
            glGetUniformIndices(context->material->program, 1, mvp_names, mvp_indices);
            glGetActiveUniformsiv(context->material->program, 3, lights_indices, GL_UNIFORM_OFFSET, context->lightdata.lights_offset);
            glGetActiveUniformsiv(context->material->program, 1, mvp_indices, GL_UNIFORM_OFFSET, context->lightdata.mvp_offset);
            context->lightdata.created = 1;
        }
        if (context->lightdata.lights_ubo) {
            glDeleteBuffers(1, &context->lightdata.lights_ubo);
            glDeleteBuffers(1, &context->lightdata.mvp_ubo);
        }
        glGenBuffers(1, &context->lightdata.lights_ubo);
        glGenBuffers(1, &context->lightdata.mvp_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, context->lightdata.lights_ubo);
        GLubyte *lights_buf = calloc(1, context->lightdata.lights_size);        
        unsigned int i;
        for (i = 0; i < context->lights.length; i++) {
            ((il_Vector3*)lights_buf + context->lightdata.lights_offset[0])[i] = context->lights.data[i]->positionable->position;
            ((il_Vector3*)lights_buf + context->lightdata.lights_offset[1])[i] = context->lights.data[i]->color;
            ((float*)lights_buf + context->lightdata.lights_offset[2])[i] = context->lights.data[i]->radius;
        }
        glBufferData(GL_UNIFORM_BUFFER, context->lightdata.lights_size, lights_buf, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, context->lightdata.mvp_ubo);
        free(lights_buf);
        GLubyte *mvp_buf = calloc(1, context->lightdata.mvp_size);
        for (i = 0; i < context->lights.length; i++) {
            ((il_Matrix*)mvp_buf + context->lightdata.mvp_offset[0])[i] = 
                il_Matrix_transpose(ilG_computeMVP(context->camera, context->lights.data[i]->positionable));
        }
        glBufferData(GL_UNIFORM_BUFFER, context->lightdata.mvp_size, mvp_buf, GL_DYNAMIC_DRAW);
        free(mvp_buf);
        context->lightdata.invalidated = 0;
    }*/
    //glBindBufferBase(GL_UNIFORM_BUFFER, context->lightdata.lights_index, context->lightdata.lights_ubo);
    //glBindBufferBase(GL_UNIFORM_BUFFER, context->lightdata.mvp_index, context->lightdata.mvp_ubo);
    context->drawable = ilG_icosahedron(context);
    context->drawableb = il_cast(il_typeof(context->drawable), "il.graphics.bindable"),
    context->materialb = il_cast(il_typeof(context->material), "il.graphics.bindable");
    ilG_bindable_bind(context->drawableb, context->drawable);
    ilG_bindable_bind(context->materialb, context->material);
    glUniform3f(glGetUniformLocation(context->material->program, "camera"), 
            context->camera->positionable.position[0], 
            context->camera->positionable.position[1],
            context->camera->positionable.position[2]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, context->fbtextures[0]);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_RECTANGLE, context->fbtextures[2]);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_RECTANGLE, context->fbtextures[3]);
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_RECTANGLE, context->fbtextures[4]);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    //glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_FRONT);
    
    GLint position_loc  = glGetUniformLocation(context->material->program, "position"),
          color_loc     = glGetUniformLocation(context->material->program, "color"),
          radius_loc    = glGetUniformLocation(context->material->program, "radius");
    //il_mat vp = ilG_computeMVP(ILG_VP, context->camera, NULL);
    //il_vec4 vec = il_vec4_new();
    unsigned int i;
    for (i = 0; i < context->lights.length; i++) {
        context->positionable = &context->lights.data[i]->positionable;
        ilG_bindable_action(context->materialb, context->material);
        il_vec3 pos = context->positionable->position;
        //pos = il_mat_mulv(vp, pos, pos);
        glUniform3f(position_loc, pos[0], pos[1], pos[2]);
        il_vec3 col = context->lights.data[i]->color;
        glUniform3f(color_loc, col[0], col[1], col[2]);
        glUniform1f(radius_loc, context->lights.data[i]->radius);
        ilG_bindable_action(context->drawableb, context->drawable);
        //glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(GLuint), GL_UNSIGNED_INT, NULL);
    }
    //il_vec4_free(vec);
    //glDrawElementsInstanced(GL_TRIANGLES, sizeof(indices)/sizeof(GLuint), GL_UNSIGNED_INT, NULL, context->lights.length);
    glDisable(GL_BLEND);
    ilG_bindable_unbind(context->drawableb, context->drawable);
    ilG_bindable_unbind(context->materialb, context->material);
    context->drawable = NULL;
    context->material = NULL;
    context->drawableb = NULL;
    context->materialb = NULL;
    ilG_testError("Error drawing lights");
}

struct ilG_stage *ilG_lightpass(struct ilG_context* context)
{
    struct lightpass *self = il_new(&ilG_lightpass_type);

    self->stage.context = context;
    self->stage.run = draw_lights;
    self->stage.name = "Deferred Shading Pass";

    // shader creation
    struct ilG_material* mtl = ilG_material_new();
    ilG_material_vertex_file(mtl, "light.vert");
    ilG_material_fragment_file(mtl, "light.frag");
    ilG_material_name(mtl, "Deferred Shader");
    ilG_material_arrayAttrib(mtl, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_textureUnit(mtl, ILG_TUNIT_NONE, "depth");
    ilG_material_textureUnit(mtl, ILG_TUNIT_NONE, "normal");
    ilG_material_textureUnit(mtl, ILG_TUNIT_NONE, "diffuse");
    ilG_material_textureUnit(mtl, ILG_TUNIT_NONE, "specular");
    ilG_material_matrix(mtl, ILG_INVERSE | ILG_VP, "ivp");
    ilG_material_fragData(mtl, ILG_FRAGDATA_ACCUMULATION, "out_Color");
    ilG_material_matrix(mtl, ILG_MODEL_T | ILG_VP, "mvp");
    ilG_material_bindFunc(mtl, size_uniform, context, "size");
    if (ilG_material_link(mtl, context)) {
        return NULL;
    }
    self->material = mtl;
    self->invalidated = 1;

    return &self->stage;
}

