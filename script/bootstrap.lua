local oldprint=print
function _G.print(...)
    local t = {...}
    local s = tostring(t[1])
    for i = 2, #t do
        s = s.."\t"..tostring(t[i])
    end
    oldprint(s)
end

local ffi = require "ffi"

function _G.loadmod(name)
    local sname = name:gsub('il(.*)', '%1'):gsub('lib(.*)', '%1')
    print('load '..name, sname)
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

