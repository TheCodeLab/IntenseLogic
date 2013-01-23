package.path = package.path..";script/?.lua;script/graphics/?.lua;script/common/?.lua"
local script = require "script"
require "graphics"
local positionable = require "positionable"
local world = require "world"
local matrix = require "matrix"
local vector3 = require "vector3"
local mesh = require "mesh"
local event = require "event"
local input = require "input"

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
box.drawable = mesh("teapot.obj"); --drawable.box;
print(box.drawable.ptr);
box.material = material.default;
box.texture = texture.fromfile "test.png"; --texture.default;
box:track(c);
print(box);

c.camera = camera(positionable(w));
c.camera.projection_matrix = matrix.perspective(75, 4/3, 0.25, 100);
c.camera.positionable.position = vector3(0, 5, 25);
c.camera.sensitivity = .2

local first_mouse = true
function mousemove(q, ev)
    if first_mouse then first_mouse = false return end
    -- do something
end

function tick(q, ev)
    local x = input.isKeySet("D") - input.isKeySet("A");
    local z = input.isKeySet("W") - input.isKeySet("S");
    local y = input.isKeySet("R") - input.isKeySet("F");

    local v = vector3(x,y,z);
    v = v * c.camera.sensitivity;

    c.camera.positionable.position = c.camera.positionable.position + v;
end

event.register(event.mainqueue, 1, tick); -- tick
event.register(event.mainqueue, 7, mousemove) -- mousemove

