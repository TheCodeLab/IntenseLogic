package.path = package.path..";script/?.lua;script/graphics/?.lua;script/common/?.lua"
local script = require "script"
local drawable = require "drawable";
local context  = require "context";
local texture  = require "texture";
local material = require "material";
local camera   = require "camera";
local positionable = require "positionable"
local world = require "world"
local matrix = require "matrix"
local vector3 = require "vector3"
local mesh = require "mesh"
local event = require "event"
local input = require "input"
local quaternion = require "quaternion"

local oldprint=print
function _G.print(...)
    local t = {...}
    local s = tostring(t[1])
    for i = 2, #t do
        s = s.."\t"..tostring(t[i])
    end
    oldprint(s)
end

local w = world();
local c = context();
c.world = w;
w.context = c;
c:setActive();
local m;
local t = texture.fromfile "white-marble-texture.png";
local vf, ff = io.open("shaders/test.vert", "r"), io.open("shaders/test.frag", "r");
local mtl = material(vf:read "*a", ff:read "*a", "test material", "in_Position", "in_Texcoord", nil, "mvp", {"tex"}, {1});
print(mtl);
if false then
    m = mesh "minecraft.obj"
    print(m.ptr)
    local minecraft = positionable()
    w:add(minecraft)
    minecraft.drawable = m;
    minecraft.material = mtl;
    minecraft.texture = texture.default;
    minecraft:track(c);
else
    m = mesh "teapot.obj"
    local width = 1
    for i = 0, width*width*width-1 do
        local box = positionable();
        w:add(box);
        box.drawable = m; --drawable.box;
        box.material = mtl;
        box.texture = t
        box.position = vector3(i % width, math.floor((i%(width*width)) / width), math.floor(i/(width*width))) * 15
        box:track(c);
    end
end
c.camera = camera(positionable(w));
c.camera.projection_matrix = matrix.perspective(75, 4/3, 0.25, 1000);
c.camera.positionable.position = vector3(0, -5, -25);
c.camera.sensitivity = .01
c.camera.movespeed = vector3(1,1,1)

local first_mouse = true
function mousemove(q, ev)
    if first_mouse then first_mouse = false return end
    if not input.isButtonSet(0) == 1 then return end -- TODO: Make this work
    local x, y = ev:unpack()
    local yaw = quaternion(vector3(0, 1, 0), x * c.camera.sensitivity);
    local pitch = quaternion(vector3(1, 0, 0), y * c.camera.sensitivity);
    c.camera.positionable.rotation = c.camera.positionable.rotation * yaw * pitch;
end

function tick(q, ev)
    local x = input.isKeySet("D") - input.isKeySet("A");
    local z = input.isKeySet("W") - input.isKeySet("S");
    local y = input.isKeySet("R") - input.isKeySet("F");
    local r = input.isKeySet("Q") - input.isKeySet("E");
    local v = vector3(x,y,z);
    v = v * c.camera.movespeed;
    c.camera.positionable.position = c.camera.positionable.position + v * c.camera.positionable.rotation;
    local bank = quaternion(vector3(0, 0, 1), r * c.camera.sensitivity);
    c.camera.positionable.rotation = c.camera.positionable.rotation * bank;
end

event.register(event.mainqueue, 1, tick); -- tick
event.register(event.mainqueue, 7, mousemove) -- mousemove

