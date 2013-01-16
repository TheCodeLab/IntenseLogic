package.path = package.path..";script/?.lua;script/graphics/?.lua;script/common/?.lua"
local script = require "script"
require "graphics"
local positionable = require "positionable"
local world = require "world"
local matrix = require "matrix"
local vector3 = require "vector3"

local oldprint=print
function _G.print(...)
    local t = {...}
    local s = ""
    for i = 1, #t do
        s = s..tostring(t[i])
    end
    oldprint(s)
end

local w = world();
local c = context();
c.world = w;
w.context = c;
c:setActive();
local box = positionable();
w:add(box);
print(box);
box.drawable = drawable.box;
print(box.drawable.ptr);
box.material = material.default;
box.texture = texture.default;
box:track(c);
print(box);

c.camera = camera(positionable(w));
c.camera.projection_matrix = matrix.perspective(75, 4/3, 0.25, 100);
c.camera.positionable.position = vector3(0, 0, 6);

