local ffi = require "ffi";

local base = require "base"

local context;

ffi.cdef [[

typedef struct il_world {
    il_base base;
    struct {
        struct il_positionable* data;
        size_t length;
        size_t capacity;
    } objects;
    struct ilG_context* context;
} il_world;

void il_world_add(il_world*, struct il_positionable*);

extern il_type il_world_type;

]]

base.wrap "il.common.world" {
    add = ffi.C.il_world_add,
    struct = "il_world"
}

return ffi.C.il_world_type

