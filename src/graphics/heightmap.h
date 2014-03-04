#ifndef ILG_HEIGHTMAP_H
#define ILG_HEIGHTMAP_H

struct ilG_context;
struct ilG_tex;

extern const struct ilG_renderable ilG_heightmap_renderer;

#define ilG_heightmap_wrap(p) ilG_renderer_wrap(p, &ilG_heightmap_renderer)

struct ilG_renderer ilG_heightmap_new(unsigned w, unsigned h, struct ilG_tex height, struct ilG_tex normal, struct ilG_tex color);

#endif

