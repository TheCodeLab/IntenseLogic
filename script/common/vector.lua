local storage
local ffi = require 'ffi'

local vector = {}

ffi.metatype('il_vector', {
    __index = vector,
    __newindex = function(t, k, v)
        assert(type(k) == "number")
        if not storage then storage = require 'common.storage' end
        if not ffi.istype('il_value', v) then
            v = storage.pack(v)
        end
        modules.common.il_vector_set(t, k, ffi.gc(v, nil))
    end
})

function vector.get(self, k)
    assert(type(k) == "number")
    if not storage then storage = require 'common.storage' end
    return storage.unpack(modules.common.il_vector_get(self, k))
end

function vector.create(...)
    if not storage then storage = require 'common.storage' end
    local n = select('#', ...)
    local t = {...}
    for i, v in pairs(t) do
        if not ffi.istype("il_value", v) then
            t[i] = storage.pack(v)
        end
    end
    local args = ffi.new('il_value[?]', n, t)
    return ffi.gc(modules.common.il_value_vector(modules.common.il_vector_newv(n, args)), modules.common.il_value_free)
end

setmetatable(vector, {__call = function(self, ...) return vector.create(...) end})

return vector

