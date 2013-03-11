package.path = package.path..";script/?.lua;script/graphics/?.lua;script/common/?.lua"

require "strict"

local script = require "script"
local drawable = require "drawable";
local context  = require "context";
local texture  = require "texture";
local material = require "material";
local camera   = require "camera";
local positionable = require "positionable"
local world = require "world"
local matrix = require "matrix"
local vector4 = require "vector4"
local mesh = require "mesh"
local event = require "event"
local input = require "input"
local quaternion = require "quaternion"
local light = require "light"

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
local c = context(800, 600);
c.world = w;
w.context = c;
c:setActive();
local m;
local t = texture()
t:fromfile("color0", "white-marble-texture.png")
local vf, ff = io.open("shaders/test.vert", "r"), io.open("shaders/test.frag", "r");
--local mtl = material(vf:read "*a", ff:read "*a", "test material", "in_Position", "in_Texcoord", "in_Normal", "mvp", {"tex"}, {1}, "out_Normal", "out_Ambient", "out_Diffuse", "out_Specular", "phong");
local mtl = material()
mtl:vertex(vf:read "*a")
mtl:fragment(ff:read "*a")
vf:close()
ff:close()
mtl:name "Test material"
mtl:arrayAttrib("position", "in_Position")
mtl:arrayAttrib("texcoord", "in_Texcoord")
mtl:arrayAttrib("normal", "in_Normal")
mtl:matrix("MVP", "mvp")
mtl:matrix("IM", "im")
mtl:textureUnit("color0", "tex")
mtl:fragData("normal", "out_Normal")
mtl:fragData("accumulation", "out_Ambient")
mtl:fragData("diffuse", "out_Diffuse")
mtl:fragData("specular", "out_Specular")
mtl:link()
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
        box.position = vector4(i % width, math.floor((i%(width*width)) / width), math.floor(i/(width*width))) * vector4(15, 15, 15, 1)
        box:track(c);
        --print(box.position)
    end
end
c.camera = camera(positionable(w));
c.camera.projection_matrix = matrix.perspective(75, 4/3, 2, 1000);
c.camera.positionable.position = vector4(0, -5, -25);
c.camera.sensitivity = .01
c.camera.movespeed = vector4(1,1,1)
--[[local l = light(-5, -5, -5, 50, 0, 0, 1.0) -- x y z radius r g b
print(l.positionable)
l:add(c)
local sig = positionable();
w:add(sig)
sig.position = vector3(-5, -5, -5)
sig.drawable = drawable.box;
sig.material = material.default;
sig.texture = texture.default;
sig:track(c)]]

local first_mouse = true
function mousemove(q, ev)
    if first_mouse then first_mouse = false return end
    if input.isButtonSet(0) == 0 then return end -- TODO: Make this work
    --print("old",c.camera.positionable.rotation)
    local x, y = ev:unpack()
    --print("x ",x,"y ", y)
    local yaw = quaternion(vector4(0, 1, 0), x * c.camera.sensitivity);
    --print("yaw:",yaw)
    local pitch = quaternion(vector4(1, 0, 0), y * c.camera.sensitivity);
    --print("pitch: ",pitch)
    local rot = c.camera.positionable.rotation * yaw * pitch;
    --print("rot: ", rot)
    c.camera.positionable.rotation = rot
end

function tick(q, ev)
    --print "tick"
    local x = input.isKeySet("D") - input.isKeySet("A");
    local z = input.isKeySet("W") - input.isKeySet("S");
    local y = input.isKeySet("R") - input.isKeySet("F");
    local r = input.isKeySet("Q") - input.isKeySet("E");
    local v = vector4(x,y,z);
    print("v", v)
    v = v * c.camera.movespeed;
    print("v'", v)
    v = v * c.camera.positionable.rotation
    print("rotation", c.camera.positionable.rotation)
    v.w = 0
    print("v''", v)
    print("old", c.camera.positionable.position)
    c.camera.positionable.position = c.camera.positionable.position + v;
    print("new", c.camera.positionable.position)
    local bank = quaternion(vector4(0, 0, 1), r * c.camera.sensitivity);
    c.camera.positionable.rotation = c.camera.positionable.rotation * bank;
end

event.register(event.mainqueue, 1, tick); -- tick
event.register(event.mainqueue, 7, mousemove) -- mousemove

