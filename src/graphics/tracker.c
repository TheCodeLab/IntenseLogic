#include "tracker.h"

#include <string.h>
#include <stdlib.h>

#include "common/world.h"
#include "graphics/context.h"
#include "graphics/drawable3d.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "common/array.h"
#include "common/log.h"

// Warning: This file is rather ugly as it uses basically a quintuple pointer.

IL_ARRAY(il_positionable*,  positionable_list);
IL_ARRAY(positionable_list, texture_list);
IL_ARRAY(texture_list,      material_list);
IL_ARRAY(material_list,     drawable_list);
IL_ARRAY(drawable_list,     context_list);

static context_list contexts;

void ilG_trackPositionable(ilG_context* ctx, il_positionable* self)
{
    // pedantic checks because why not
    if (!ctx) {
        il_log(1, "Null context");
        return;
    }
    if (!self) {
        il_log(1, "Null positionable");
        return;
    }
    if (!self->drawable) {
        il_log(1, "Null drawable in positionable");
        return;
    }
    if (!self->material) {
        il_log(1, "Null material in positionable");
        return;
    }
    if (!self->texture) {
        il_log(1, "Null texture in positionable");
        return;
    }

    drawable_list * drawables;
    IL_INDEXORZERO(contexts, ctx->world->id, drawables);

    material_list *materials;
    IL_INDEXORZERO(*drawables, self->drawable->id, materials);

    texture_list *textures;
    IL_INDEXORZERO(*materials, self->material->id, textures);
   
    positionable_list *positionables;
    IL_INDEXORZERO(*textures, self->texture->id, positionables);
    
    IL_APPEND(*positionables, self);

}

void ilG_untrackPositionable(ilG_context* ctx, il_positionable* positionable)
{
    // TODO: stub function
}

struct ilG_trackiterator {
    int context, drawable, material, texture, positionable;
};

ilG_trackiterator* ilG_trackiterator_new(ilG_context* ctx) 
{
    ilG_trackiterator *iter = calloc(1, sizeof(ilG_trackiterator));
    iter->context = ctx->world->id;
    iter->positionable = -1;
    return iter;
}

int ilG_trackIterate(ilG_trackiterator* iter)
{
    iter->positionable++;
    if (iter->context       >= contexts.length ||
        iter->drawable      >= contexts.data[iter->context].length ||
        iter->material      >= contexts.data[iter->context].data[iter->drawable].length ||
        iter->texture       >= contexts.data[iter->context].data[iter->drawable].data[iter->material].length ||
        iter->positionable  >= contexts.data[iter->context].data[iter->drawable].data[iter->material].data[iter->texture].length)
    {
        return 0;
    }
    if (iter->positionable >= contexts.data[iter->context].data[iter->drawable].data[iter->material].data[iter->texture].length) {
        iter->positionable = 0;
        iter->texture++;
    }
    if (iter->texture >= contexts.data[iter->context].data[iter->drawable].data[iter->material].length) {
        iter->texture = 0;
        iter->material++;
    }
    if (iter->material >= contexts.data[iter->context].data[iter->drawable].length) {
        iter->material = 0;
        iter->drawable++;
    }
    if (iter->drawable >= contexts.data[iter->context].length) {
        free(iter);
        return 0;
    }
    if (!contexts.data[iter->context].data[iter->drawable].data[iter->material].data[iter->texture].data[iter->positionable])
        return ilG_trackIterate(iter);
    return 1;
}

il_positionable* ilG_trackGetPositionable(ilG_trackiterator* iter)
{
    return contexts.data[iter->context].data[iter->drawable].data[iter->material].data[iter->texture].data[iter->positionable];
}

ilG_drawable3d* ilG_trackGetDrawable(ilG_trackiterator* iter)
{
    return ilG_drawable3d_fromId(iter->drawable);
}

ilG_material* ilG_trackGetMaterial(ilG_trackiterator* iter)
{
    return ilG_material_fromId(iter->material);
}

ilG_texture* ilG_trackGetTexture(ilG_trackiterator* iter)
{
    return ilG_texture_fromId(iter->texture);
}

IL_ARRAY(ilG_drawable3d*, drawable_index);
drawable_index drawable_indices;
IL_ARRAY(ilG_material*, material_index);
material_index material_indices;
IL_ARRAY(ilG_texture*, texture_index);
texture_index texture_indices;

ilG_drawable3d* ilG_drawable3d_fromId(unsigned int id) 
{
    ilG_drawable3d** out;
    IL_INDEX(drawable_indices, id, out);
    if (out) return *out;
    return NULL;
}

ilG_material* ilG_material_fromId(unsigned int id) 
{
    ilG_material** out;
    IL_INDEX(material_indices, id, out);
    if (out) return *out;
    return NULL;
}

ilG_texture* ilG_texture_fromId(unsigned int id)
{
    ilG_texture** out;
    IL_INDEX(texture_indices, id, out);
    if (out) return *out;
    return NULL;
}

void ilG_drawable3d_assignId(ilG_drawable3d* self)
{
    if (self->id) return; // already has an ID???
    self->id = drawable_indices.length;
    IL_APPEND(drawable_indices, self);
}

void ilG_material_assignId(ilG_material* self)
{
    if (self->id) return;
    self->id = material_indices.length;
    IL_APPEND(material_indices, self);
}

void ilG_texture_assignId(ilG_texture* self)
{
    if (self->id) return;
    self->id = texture_indices.length;
    IL_APPEND(texture_indices, self);
}

void ilG_drawable3d_setId(ilG_drawable3d* self, unsigned int id)
{
    self->id = id;
    IL_SET(drawable_indices, id, self);
}

void ilG_material_setId(ilG_material* self, unsigned int id)
{
    self->id = id;
    IL_SET(material_indices, id, self);
}

void ilG_texture_setId(ilG_texture* self, unsigned int id)
{
    self->id = id;
    IL_SET(texture_indices, id, self);
}

