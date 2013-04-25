--- Wrapper around world type
-- See `common.base` for details on interacting with this type.
-- @type world
local ffi = require "ffi";

local base = require "common.base"

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
    --- Adds a positionable to a world
    -- @tparam positionable positionable The positionable to add
    add = ffi.C.il_world_add,
    struct = "il_world"
}

return ffi.C.il_world_type

