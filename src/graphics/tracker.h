#ifndef ILG_TRACKER_H
#define ILG_TRACKER_H

struct ilG_drawable3d;
struct ilG_material;
struct ilG_texture;
struct il_positionable;
struct ilG_context;

void ilG_trackPositionable(struct ilG_context*, struct il_positionable*);
void ilG_untrackPositionable(struct ilG_context*, struct il_positionable*);

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

