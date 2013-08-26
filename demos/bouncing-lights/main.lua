require "strict"

local script        = require "script"
local drawable      = require "graphics.drawable"
local context       = require "graphics.context"
local texture       = require "graphics.texture"
local material      = require "graphics.material"
local camera        = require "graphics.camera"
local positionable  = require "common.positionable"
local world         = require "common.world"
local matrix        = require "math.matrix"
local vector3       = require "math.vector3"
local drawnmesh     = require "graphics.mesh"
local event         = require "common.event"
local input         = require "input.input"
local quaternion    = require "math.quaternion"
local light         = require "graphics.light"
local stage         = require "graphics.stage"
local outpass       = require "graphics.outpass"
local geometrypass  = require "graphics.geometrypass"
local guipass       = require "graphics.guipass"
local frame         = require "graphics.gui.frame"
local image         = require "asset.image"
local text          = require "graphics.gui.text"
local mesh          = require "asset.mesh"
local lightpass     = require "graphics.lightpass"
local file          = require "asset.file"
local heightmap     = require "graphics.heightmap"
local skyboxpass    = require "graphics.skyboxpass"
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

