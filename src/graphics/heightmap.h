#ifndef ILG_HEIGHTMAP_H
#define ILG_HEIGHTMAP_H

struct ilG_drawable3d;
struct ilG_texture;
struct il_heightmap;

struct ilG_drawable3d* ilG_heightmapDrawable(const struct il_heightmap* self);
struct ilG_texture* ilG_heightmapTexture(const struct il_heightmap* self);

#endif

