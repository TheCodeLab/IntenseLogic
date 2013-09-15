require "strict"

local texture       = require "graphics.texture"
local material      = require "graphics.material"
local positionable  = require "common.positionable"
local vector3       = require "math.vector3"
local light         = require "graphics.light"
local image         = require "asset.image"
local heightmap     = require "graphics.heightmap"
local helper        = require "demos.helper"
local event         = require "common.event"
local ffi           = require "ffi"
local drawable      = require "graphics.drawable"

ffi.cdef [[

void set_world(il_world *w);
void add_heightmap(ilA_img *hm, float height);
void add_ball(il_positionable *pos);
void update();

]]

math.randomseed(os.time())

local c, w, root = helper.context{name="Bouncing Lights Demo",skybox='demos/bouncing-lights/stars.png',geom=true,lights=true,gui=true,output=true}
ffi.C.set_world(w)

local ht = texture()
ht:setContext(c)
local hmt = image.loadfile "demos/bouncing-lights/smooth-heightmap.png"
ffi.C.add_heightmap(hmt, 50)
ht:fromimage("height0", hmt)
ht:fromimage("normal0", hmt:height_to_normal())
ht:fromfile("color0", "demos/bouncing-lights/terrain.png")
local hm = positionable()
w:add(hm)
hm.drawable = heightmap(c, 100, 100)
hm.material = heightmap.defaultShader(c)
hm.texture = ht
hm.position = vector3(0, 0, 0).ptr
hm.size = vector3(100, 50, 100).ptr
hm:track(c)

local plain = material()
plain:vertex(io.open("shaders/plain.vert","r"):read "*a")
plain:fragment(io.open("shaders/plain.frag", "r"):read "*a")
plain:mtlname "Plain material"
plain:arrayAttrib("position", "in_Position")
plain:matrix("MVP", "mvp")
plain:link(c)

local hw, hh = 100, 100
for i = 1, 100 do
    local l = light()
    local pos = vector3(math.random(0,hw-1), 0, math.random(0,hh-1))
    local height = hmt:getPixel(pos.x, pos.z) 
    pos.y = 100 --height * 50 + 2
    l.positionable.position = pos.ptr
    l.positionable.size = vector3(.25, .25, .25).ptr
    l.radius = math.random(1, 15)
    l.color = vector3(math.random(0,1), math.random(0,1), math.random(0,1)).ptr
    l:add(c)
    ffi.C.add_ball(l.positionable)
    l.positionable.drawable = drawable.box(c)
    l.positionable.material = plain
    local tex = texture()
    tex:setContext(c)
    l.positionable.texture = tex
    hm.track(l.positionable, c) --ugh
    w:add(l.positionable)
end

--event.setPacker(event.registry, "physics.tick", event.nilPacker)
--event.timer(event.registry, "physics.tick", 1/60)
event.register(event.registry, "input.button", function(reg, name, key) 
    if key == ' ' then
        event.register(event.registry, "tick", function() ffi.C.update() end)
    end
end)
helper.camera(c, root)

