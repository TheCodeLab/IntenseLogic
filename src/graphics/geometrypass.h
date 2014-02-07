#ifndef ILG_GEOMETRYPASS_H
#define ILG_GEOMETRYPASS_H

struct ilG_stage;
struct ilG_context;
struct ilG_renderer;

struct ilG_stage *ilG_geometrypass_new(struct ilG_context *context);
void ilG_geometrypass_track(struct ilG_stage *self, struct ilG_renderer *renderer);

#endif

