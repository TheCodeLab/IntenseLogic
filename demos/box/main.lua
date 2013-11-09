require "strict"

local drawable      = require "graphics.drawable"
local texture       = require "graphics.texture"
local material      = require "graphics.material"
local positionable  = require "common.positionable"
local vector3       = require "math.vector3"
local helper        = require "demos.helper"

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

c:setActive()

