#ifndef IL_GRAPHICS_SHAPE_H
#define IL_GRAPHICS_SHAPE_H

struct ilG_context;
typedef struct ilG_shape ilG_shape;

ilG_shape* ilG_box(struct ilG_context *context);
ilG_shape* ilG_icosahedron(struct ilG_context *context);
void ilG_shape_bind(ilG_shape *shape);
void ilG_shape_draw(ilG_shape *shape);

#endif
