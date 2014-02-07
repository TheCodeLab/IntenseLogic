#ifndef ILG_TRANSPARENCYPASS_H
#define ILG_TRANSPARENCYPASS_H

struct ilG_stage;
struct ilG_context;
struct ilG_renderer;

struct ilG_stage *ilG_transparencypass_new(struct ilG_context *context);
void ilG_transparencypass_track(struct ilG_stage *self, struct ilG_renderer *renderer);

#endif

