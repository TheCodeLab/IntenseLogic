--- Wrapper for positionables
-- Uses the IL object system, see `common.base` for more information
-- @type positionable
local ffi           = require "ffi"
local drawable      = require "graphics.drawable"
local material      = require "graphics.material"
local texture       = require "graphics.texture"
local world         = require "common.world"
local context       = require "graphics.context"
local vector3       = require "math.vector3"
local quaternion    = require "math.quaternion"

local positionable = {}

function positionable.create(world)
    assert(world)
    return modules.common.il_positionable_new(world)
end

setmetatable(positionable, {__call = function(self, ...) return positionable.create(...) end})

ffi.metatype("il_positionable", {
    __index = function(self, k)
        local p = function(n)
            if k == n:lower() then
                return modules.common["il_positionable_get"..n](self)
            end
            return nil
        end
        return p "Position"
        or p "Rotation"
        or p "Size"
        or p "Velocity"
        or p "LastUpdate"
        or modules.common.il_positionable_mgetStorage(self)[k]
        or positionable[k]
    end,
    __newindex = function(self, k, v)
        local p = function(n)
            if k == n:lower() then
                modules.common["il_positionable_set"..n](self, v)
                return true
            end
            return false
        end
        local _ = p "Position"
        or p "Rotation"
        or p "Size"
        or p "Velocity"
        or p "LastUpdate"
        or (function() modules.common.il_positionable_mgetStorage(self)[k] = v end)()
    end
})

return positionable

