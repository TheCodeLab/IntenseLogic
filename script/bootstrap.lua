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
-- TODO: make this not suck
function _G.loadmod(name)
    --print("load lib"..name)
    return ffi.load("lib"..name, true)
end

_G.modules = {
    util = loadmod "ilutil",
    math = loadmod "ilmath",
    common = loadmod "ilcommon",
    network = loadmod "ilnetwork",
    graphics = loadmod "ilgraphics",
    asset = loadmod "ilasset",
    input = loadmod "ilinput"
}

