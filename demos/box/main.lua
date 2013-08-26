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

local c, w, root = helper.context{name="Box Demo",geom=true,gui=true,output=true}

local plain = material()
plain:vertex(io.open("shaders/plain.vert","r"):read "*a")
plain:fragment(io.open("shaders/plain.frag", "r"):read "*a")
plain:mtlname "Plain material"
plain:arrayAttrib("position", "in_Position")
plain:matrix("MVP", "mvp")
plain:link(c)
m = drawable.box(c)
local tex = texture()
tex:setContext(c)
local box = positionable()
w:add(box)
box.drawable = m
box.material = plain
box.texture = tex
box.position = vector3(0, 0, -10).ptr
box:track(c)

helper.camera(c, root)

