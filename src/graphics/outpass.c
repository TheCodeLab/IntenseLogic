#include "outpass.h"

#include <GL/glew.h>
#include <GL/glfw.h>

#include "graphics/stage.h"
#include "graphics/context.h"
#include "graphics/material.h"
#include "graphics/glutil.h"
#include "graphics/arrayattrib.h"
#include "graphics/textureunit.h"
#include "asset/asset.h"

struct outpass {
    ilG_stage stage;
    ilG_material *material;
    GLuint vao, vbo;
    int which;
};

il_type ilG_outpass_type = {
    .typeclasses = NULL,
    .storage = NULL,
    .constructor = NULL,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.outpass",
    .registry = NULL,
    .size = sizeof(struct outpass),
    .parent = &ilG_stage_type
};

static void fullscreenTexture(struct outpass *self)
{
    ilG_testError("Unknown");
    glBindVertexArray(self->vao);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    ilG_testError("Error drawing fullscreen quad");
}


static void out_pass(ilG_stage *ptr)
{
    if (!glfwGetWindowParam(GLFW_OPENED)) { // TODO: find a better way to do this
        ilE_globalevent(il_registry, "shutdown", 0, NULL);
        return;
    }
    struct outpass *self = (struct outpass*)ptr;
    ilG_context *context = ptr->context;
    
    ilG_testError("Unknown");
    ilG_material* material = context->material = self->material;
    // prepare the GL state for outputting to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.196, 0.804, 0.196, 1.0); // lime green
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    ilG_bindable_bind(il_cast(il_typeof(material), "il.graphics.bindable"), material);
    // no action() as we don't deal with positionables here
    // setup to do postprocessing
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, context->fbtextures[self->which]); // bind the framebuffer we want to display
    ilG_testError("Error setting up for post processing");
    fullscreenTexture(self);
    ilG_testError("Error post processing");
    ilG_bindable_unbind(il_cast(il_typeof(material), "il.graphics.bindable"), material);
    context->material = NULL;
    ilG_testError("Error cleaning up shaders");
    glfwSwapBuffers();
}

struct ilG_stage *ilG_outpass(struct ilG_context *context)
{
    struct outpass *self = il_new(&ilG_outpass_type);
    
    self->stage.context = context;
    self->stage.run = out_pass;
    self->stage.name = "Screen Output";
    
    ilG_material *material = ilG_material_new();
    ilG_material_vertex(material, IL_ASSET_READFILE("post.vert"));
    ilG_material_fragment(material, IL_ASSET_READFILE("post.frag"));
    ilG_material_name(material, "Post Processing Shader");
    ilG_material_arrayAttrib(material, ILG_ARRATTR_POSITION, "in_Position");
    ilG_material_arrayAttrib(material, ILG_ARRATTR_TEXCOORD, "in_Texcoord");
    ilG_material_textureUnit(material, ILG_TUNIT_NONE, "tex");
    if (ilG_material_link(material, context)) {
        abort();
    }
    ilG_testError("Error creating material");
    self->material = material;

    static const float data[] = {
        0, 0,
	0, 1,
	1, 1,
	1, 0
    };
    glGenVertexArrays(1, &self->vao);
    glBindVertexArray(self->vao);
    glGenBuffers(1, &self->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
    glVertexAttribPointer(ILG_ARRATTR_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribPointer(ILG_ARRATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(ILG_ARRATTR_POSITION);
    glEnableVertexAttribArray(ILG_ARRATTR_TEXCOORD);

    self->which = 1;

    return &self->stage;
}

