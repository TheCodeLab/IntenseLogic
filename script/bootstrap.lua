local oldprint=print
function _G.print(...)
    local t = {...}
    local t2 = {}
    for _, v in pairs(t) do
        t2[#t2+1] = tostring(v)
    end
    oldprint(table.concat(t2, '\t'))
end

local ffi = require "ffi"

function _G.loadmod(name)
    local sname = name:gsub('il(.*)', '%1'):gsub('lib(.*)', '%1')
    modules[sname] = ffi.load("lib"..name, true)
    return modules[sname]
end

_G.modules = {}

local saveptr = iterate_modules()
while true do
    local mod = iterate_modules(saveptr)
    if not mod then break end
    loadmod(mod)
end

