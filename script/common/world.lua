--- Wrapper around world type
-- See `common.base` for details on interacting with this type.
-- @type world
local ffi = require "ffi";

local storage = require 'common.storage'

require 'math.scalar_defs'

ffi.cdef [[

typedef struct il_positionable {
    struct il_world *world;
    size_t id;
} il_positionable;

typedef struct il_world il_world;

il_world* il_world_new(size_t prealloc);
void il_world_free(il_world *self);

size_t il_world_numPositionables(const il_world *self);
il_positionable il_world_iter(il_world *self);
il_positionable il_world_next(const il_positionable *self);
const il_table *il_world_getStorage(const il_world *self);
il_table *il_world_mgetStorage(il_world *self);

il_positionable il_positionable_new(il_world *self);
void il_positionable_destroy(il_positionable self);

il_world *il_positionable_getWorld(const il_positionable *self);
il_vec3 il_positionable_getPosition(const il_positionable *self);
il_quat il_positionable_getRotation(const il_positionable *self);
il_vec3 il_positionable_getSize(const il_positionable *self);
il_vec3 il_positionable_getVelocity(const il_positionable *self);
struct timeval il_positionable_getLastUpdate(const il_positionable *self);
const il_table *il_positionable_getStorage(const il_positionable *self);
il_table *il_positionable_mgetStorage(il_positionable *self);

void il_positionable_setPosition(il_positionable *self, il_vec3 pos);
void il_positionable_setRotation(il_positionable *self, il_quat rot);
void il_positionable_setSize(il_positionable *self, il_vec3 size);
void il_positionable_setVelocity(il_positionable *self, il_vec3 vel);
void il_positionable_setLastUpdate(il_positionable *self, struct timeval tv);

]]

local world = {}

function world:iter()
    local pos = modules.common.il_world_iter(self)
    local i = 0
    return function()
        i = i + 1
        if i < modules.common.il_world_numPositionables(self) then
            pos = modules.common.il_world_next(pos)
            return pos
        end
    end
end

function world.create()
    return modules.common.il_world_new(64)
end

setmetatable(world, {__call = function(self, ...) return world.create(...) end})

ffi.metatype("il_world", {
    __index = function(self, k)
        local res = modules.common.il_world_mgetStorage(self)[k]
        if res == nil then
            res = world[k]
        end
        return res
    end,
    __newindex = function(self, k, v)
        modules.common.il_world_mgetStorage(self)[k] = v
    end,
    __len = function(self)
        return modules.common.il_world_numPositionables(self)
    end
})

return world

