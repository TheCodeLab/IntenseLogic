#ifndef ILG_HEIGHTMAP_H
#define ILG_HEIGHTMAP_H

struct ilG_context;

struct ilG_drawable3d *ilG_heightmap_new(struct ilG_context *context, unsigned w, unsigned h);
struct ilG_material *ilG_heightmap_shader(struct ilG_context *context);

#endif

