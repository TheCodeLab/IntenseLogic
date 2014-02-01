#ifndef ILG_TRACKER_H
#define ILG_TRACKER_H

#include "common/world.h"

struct ilG_drawable3d;
struct ilG_material;
struct ilG_texture;
struct ilG_context;

typedef struct ilG_tracker {
    il_positionable pos;
    struct ilG_drawable3d *drawable;
    struct ilG_material *material;
    struct ilG_texture *texture;
} ilG_tracker;

// -1 on failure
int ilG_track(struct ilG_context*, ilG_tracker);
void ilG_untrack(struct ilG_context*, int id);

typedef struct ilG_trackiterator ilG_trackiterator;
ilG_trackiterator* ilG_trackiterator_new(struct ilG_context*);
int ilG_trackIterate(ilG_trackiterator*);
struct il_positionable* ilG_trackGetPositionable(ilG_trackiterator*);
struct ilG_drawable3d* ilG_trackGetDrawable(ilG_trackiterator*);
struct ilG_material* ilG_trackGetMaterial(ilG_trackiterator*);
struct ilG_texture* ilG_trackGetTexture(ilG_trackiterator*);

struct ilG_drawable3d* ilG_drawable3d_fromId(unsigned int id);
struct ilG_material* ilG_material_fromId(unsigned int id);
struct ilG_texture* ilG_texture_fromId(unsigned int id);

void ilG_drawable3d_assignId(struct ilG_drawable3d* self);
void ilG_material_assignId(struct ilG_material* self);
void ilG_texture_assignId(struct ilG_texture* self);

void ilG_drawable3d_setId(struct ilG_drawable3d* self, unsigned int id);
void ilG_material_setId(struct ilG_material* self, unsigned int id);
void ilG_texture_setId(struct ilG_texture* self, unsigned int id);

#endif

