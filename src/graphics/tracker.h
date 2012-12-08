#ifndef ILG_TRACKER_H
#define ILG_TRACKER_H

struct ilG_drawable3d;
struct ilG_material;
struct ilG_texture;
struct il_positionable;
struct ilG_context;

void ilG_trackPositionable(struct ilG_context*, struct il_positionable*, 
        struct ilG_drawable3d*, struct ilG_material*, struct ilG_texture*);
void ilG_untrackPositionable(struct ilG_context*, struct il_positionable*);

typedef struct ilG_trackiterator ilG_trackiterator;
ilG_trackiterator* ilG_trackiterator_new(struct ilG_context*);
int ilG_trackIterate(ilG_trackiterator*);
struct il_positionable* ilG_trackGetPositionable(ilG_trackiterator*);
struct ilG_drawable3d* ilG_trackGetDrawable(ilG_trackiterator*);
struct ilG_material* ilG_trackGetMaterial(ilG_trackiterator*);
struct ilG_texture* ilG_trackGetTexture(ilG_trackiterator*);

#endif

