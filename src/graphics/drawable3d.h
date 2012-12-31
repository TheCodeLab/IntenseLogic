#ifndef IL_GRAPHICS_DRAWABLE3D_H
#define IL_GRAPHICS_DRAWABLE3D_H

struct il_positionable;
struct ilG_context;

typedef void (*ilG_drawable3d_cb)(struct ilG_context*, struct il_positionable*,
    void*);
typedef void (*ilG_drawable3d_bind_cb)(struct ilG_context*, void*);
typedef void (*ilG_drawable3d_update_cb)(struct ilG_context*, 
    struct il_positionable*, void*);

typedef struct ilG_drawable3d {
  const char* name;
  unsigned int id;
  ilG_drawable3d_cb draw;
  ilG_drawable3d_bind_cb bind, unbind;
  ilG_drawable3d_update_cb update;
  void *draw_ctx, *bind_ctx, *unbind_ctx, *update_ctx;
  unsigned long long attrs;
} ilG_drawable3d;

ilG_drawable3d* ilG_drawable3d_fromId(unsigned int id); // tracker.c
void ilG_drawable3d_assignId(ilG_drawable3d*);
void ilG_drawable3d_setId(ilG_drawable3d*, unsigned int id);

#endif

