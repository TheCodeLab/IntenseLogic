#include "tracker.h"

#include <string.h>
#include <stdlib.h>

#include "common/world.h"
#include "graphics/context.h"
#include "graphics/drawable3d.h"
#include "graphics/material.h"
#include "graphics/texture.h"

// Warning: This file is rather ugly as it uses basically a quintuple pointer.

#define LIST(T, name)       \
    struct name {           \
        T *data;            \
        size_t length;      \
        size_t capacity;    \
    }
#define RESIZE(list)                            \
    {                                           \
        size_t newcap = (list).capacity * 2;    \
        if (newcap < 50) newcap = 50;           \
        size_t size = sizeof((list).data[0]);   \
        void *tmp = calloc(size, newcap);       \
        if ((list).data) {                      \
            memcpy(tmp, (list).data,            \
                size * (list).length);          \
            free((list).data);                  \
        }                                       \
        (list).data = tmp;                      \
    }
#define INDEX(list, id, out)            \
    {                                   \
        if ((id) < (list).length) {     \
            (out) = (list).data + id;   \
        } else (out) = NULL;            \
    }
#define SET(list, id, in) \
    {                                           \
        if ((id) < (list).length)               \
            (list).data[id] = (in);             \
        else {                                  \
            while ((id) <= (list).capacity) {   \
                RESIZE(list);                   \
            }                                   \
            (list).length = (id) + 1;           \
            (list).data[id] = (in);             \
        }                                       \
    }
#define INDEXORSET(list, id, out, in) \
    {                           \
        INDEX(list, id, out);   \
        if (!(out)) {           \
            SET(list, id, in);  \
            (out) = &(in);      \
        }                       \
    }
#define INDEXORZERO(list, id, out)          \
    {                                       \
        INDEX(list, id, out);               \
        if (!(out)) {                       \
            while ((list).capacity <= (id)) \
                RESIZE(list);               \
            memset((list).data + id, 0,     \
                    sizeof((list).data[0]));\
            (out) = (list).data + id;       \
        }                                   \
    }
#define APPEND(list, in)                        \
    {                                           \
        if ((list).length >= (list).capacity) { \
            RESIZE(list);                       \
        }                                       \
        (list).data[(list).length] = (in);      \
        (list).length++;                        \
    }

LIST(il_positionable*, positionable_list);
LIST(struct positionable_list, texture_list);
LIST(struct texture_list, material_list);
LIST(struct material_list, drawable_list);
LIST(struct drawable_list, context_list);

static struct context_list context_list;

void ilG_trackPositionable(ilG_context* ctx, il_positionable* self, 
    ilG_drawable3d* drawable, ilG_material* material, ilG_texture* texture)
{
    struct drawable_list * drawable_list;
    INDEXORZERO(context_list, ctx->world->id, drawable_list);

    struct material_list *material_list;
    INDEXORZERO(*drawable_list, drawable->id, material_list);

    struct texture_list *texture_list;
    INDEXORZERO(*material_list, material->id, texture_list);
   
    struct positionable_list *positionable_list;
    INDEXORZERO(*texture_list, texture->id, positionable_list);
    
    APPEND(*positionable_list, self);

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
    if (iter->context       >= context_list.length ||
        iter->drawable      >= context_list.data[iter->context].length ||
        iter->material      >= context_list.data[iter->context].data[iter->drawable].length ||
        iter->texture       >= context_list.data[iter->context].data[iter->drawable].data[iter->material].length ||
        iter->positionable  >= context_list.data[iter->context].data[iter->drawable].data[iter->material].data[iter->texture].length)
    {
        return 0;
    }
    if (iter->positionable >= context_list.data[iter->context].data[iter->drawable].data[iter->material].data[iter->texture].length) {
        iter->positionable = 0;
        iter->texture++;
    }
    if (iter->texture >= context_list.data[iter->context].data[iter->drawable].data[iter->material].length) {
        iter->texture = 0;
        iter->material++;
    }
    if (iter->material >= context_list.data[iter->context].data[iter->drawable].length) {
        iter->material = 0;
        iter->drawable++;
    }
    if (iter->drawable >= context_list.data[iter->context].length) {
        free(iter);
        return 0;
    }
    if (!context_list.data[iter->context].data[iter->drawable].data[iter->material].data[iter->texture].data[iter->positionable])
        return ilG_trackIterate(iter);
    return 1;
}

il_positionable* ilG_trackGetPositionable(ilG_trackiterator* iter)
{
    return context_list.data[iter->context].data[iter->drawable].data[iter->material].data[iter->texture].data[iter->positionable];
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

LIST(ilG_drawable3d*, drawable_index) drawable_index;
LIST(ilG_material*, material_index) material_index;
LIST(ilG_texture*, texture_index) texture_index;

ilG_drawable3d* ilG_drawable3d_fromId(unsigned int id) 
{
    ilG_drawable3d** out;
    INDEX(drawable_index, id, out);
    if (out) return *out;
    return NULL;
}

ilG_material* ilG_material_fromId(unsigned int id) 
{
    ilG_material** out;
    INDEX(material_index, id, out);
    if (out) return *out;
    return NULL;
}

ilG_texture* ilG_texture_fromId(unsigned int id)
{
    ilG_texture** out;
    INDEX(texture_index, id, out);
    if (out) return *out;
    return NULL;
}

void ilG_drawable3d_assignId(ilG_drawable3d* self)
{
    if (self->id) return; // already has an ID???
    self->id = drawable_index.length;
    APPEND(drawable_index, self);
}

void ilG_material_assignId(ilG_material* self)
{
    if (self->id) return;
    self->id = material_index.length;
    APPEND(material_index, self);
}

void ilG_texture_assignId(ilG_texture* self)
{
    if (self->id) return;
    self->id = texture_index.length;
    APPEND(texture_index, self);
}

void ilG_drawable3d_setId(ilG_drawable3d* self, unsigned int id)
{
    self->id = id;
    SET(drawable_index, id, self);
}

void ilG_material_setId(ilG_material* self, unsigned int id)
{
    self->id = id;
    SET(material_index, id, self);
}

void ilG_texture_setId(ilG_texture* self, unsigned int id)
{
    self->id = id;
    SET(texture_index, id, self);
}

