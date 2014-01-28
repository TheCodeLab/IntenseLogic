require "strict"

local texture       = require "graphics.texture"
local material      = require "graphics.material"
local positionable  = require "common.positionable"
local vector3       = require "math.vector3"
local drawnmesh     = require "graphics.mesh"
local light         = require "graphics.light"
local helper        = require "demos.helper"

local c, w, root = helper.context{name="Teapot Demo",geom=true,lights=true,gui=true,output=true}

local marble = texture()
marble:setContext(c)
marble:fromfile("color0", "white-marble-texture.png")

local vf, ff = io.open("shaders/test.vert", "r"), io.open("shaders/test.frag", "r");
local mtl = material()
mtl:vertex(vf:read "*a")
mtl:fragment(ff:read "*a")
vf:close()
ff:close()
mtl:mtlname "Test material"
mtl:arrayAttrib("position", "in_Position")
mtl:arrayAttrib("texcoord", "in_Texcoord")
mtl:arrayAttrib("normal", "in_Normal")
mtl:arrayAttrib("diffuse", "in_Diffuse")
mtl:arrayAttrib("specular", "in_Specular")
mtl:matrix("MVP", "mvp")
mtl:matrix("IMT", "imt")
mtl:textureUnit("color0", "tex")
mtl:fragData("normal", "out_Normal")
mtl:fragData("accumulation", "out_Ambient")
mtl:fragData("diffuse", "out_Diffuse")
mtl:fragData("specular", "out_Specular")
mtl:link(c)
local m = drawnmesh "demos/teapots/teapot.obj"
local width = 3
for i = 0, width*width*width-1 do
    local box = positionable()
    w:add(box)
    box.drawable = m --drawable.box
    box.material = mtl
    box.texture = marble
    box.position = (vector3(i % width, math.floor((i%(width*width)) / width), math.floor(i/(width*width))) * vector3(15, 15, 15)).ptr
    box:track(c)
    --print(box.position)
end

lights = {
    {vector3(50, 50, 50),   250,    vector3(.4, .4, .4)},
    {vector3(40, 5, 55),    25,     vector3(.3, .4, 1)},
    {vector3(70, 20, 40),   10,     vector3(1, .5, .2)},
    {vector3(20, 5, 60),    20,     vector3(.8, .7, .1)},
}

for _, v in pairs(lights) do
    local l = light()
    l.positionable.position = v[1].ptr
    l.radius = v[2]
    l.color = v[3].ptr
    l:add(c)
end

helper.camera(c, root)

c:start()

