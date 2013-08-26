require "strict"

local texture       = require "graphics.texture"
local material      = require "graphics.material"
local positionable  = require "common.positionable"
local vector3       = require "math.vector3"
local light         = require "graphics.light"
local image         = require "asset.image"
local heightmap     = require "graphics.heightmap"
local helper        = require "demos.helper"

math.randomseed(os.time())

local c, w, root = helper.context{name="Bouncing Lights Demo",skybox='demos/bouncing-lights/stars.png',geom=true,lights=true,gui=true,output=true}

local ht = texture()
ht:setContext(c)
local hmt = image.loadfile "demos/bouncing-lights/smooth-heightmap.png"
ht:fromimage("height0", hmt)
ht:fromimage("normal0", hmt:height_to_normal())
ht:fromfile("color0", "white-marble-texture.png")
local hm = positionable()
w:add(hm)
hm.drawable = heightmap(c, 100, 100)
hm.material = heightmap.defaultShader(c)
hm.texture = ht
hm.position = vector3(0, 0, 0).ptr
hm.size = vector3(100, 50, 100).ptr
hm:track(c)

local hw, hh = 100, 100
for i = 1, 100 do
    local l = light()
    local pos = vector3(math.random(0,hw-1), 0, math.random(0,hh-1))
    local height = hmt:getPixel(pos.x, pos.z) 
    pos.y = height * 50 + 2
    l.positionable.position = pos.ptr
    l.radius = math.random(1, 15)
    l.color = vector3(math.random(0,1), math.random(0,1), math.random(0,1)).ptr
    l:add(c)
end

helper.camera(c, root)

