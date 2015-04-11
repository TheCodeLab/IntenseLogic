#ifndef IL_GRAPHICS_SHAPE_H
#define IL_GRAPHICS_SHAPE_H

struct ilG_renderman;
typedef struct ilG_shape ilG_shape;

ilG_shape* ilG_box(struct ilG_renderman *rm);
ilG_shape* ilG_icosahedron(struct ilG_renderman *rm);
void ilG_shape_bind(ilG_shape *shape);
void ilG_shape_draw(ilG_shape *shape);

#endif
