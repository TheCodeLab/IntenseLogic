local ffi = require "ffi"

ffi.cdef [[

typedef struct ilG_gui_coord {
    int x, y;
    float xp, yp;
} ilG_gui_coord;

typedef struct ilG_gui_rect {
    ilG_gui_coord a, b;
} ilG_gui_rect;

]]

