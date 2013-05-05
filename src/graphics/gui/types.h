#ifndef ILG_TYPES_H
#define ILG_TYPES_H

typedef struct ilG_gui_coord {
    int x, y;
    float xp, yp;
} ilG_gui_coord;

typedef struct ilG_gui_rect {
    ilG_gui_coord a, b;
} ilG_gui_rect;

#endif

