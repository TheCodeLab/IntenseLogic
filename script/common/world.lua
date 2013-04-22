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

ffi.metatype("il_world", base.metatable)

local T = ffi.C.il_world_type
T.struct = "il_world"
T.add = ffi.C.il_world_add
return ffi.C.il_world_type

