#ifndef ILG_GUI_QUAD_H
#define ILG_GUI_QUAD_H

struct ilG_context;

typedef struct ilG_quad ilG_quad;

ilG_quad *ilG_quad_get(struct ilG_context *context);
void ilG_quad_bind(ilG_quad *self);
void ilG_quad_draw(ilG_quad *self);

#endif
