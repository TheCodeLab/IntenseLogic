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
local camera        = require "demos.bouncing-lights.camera"
local drawnmesh     = require "graphics.mesh"

ffi.cdef [[

void set_world(il_world *w);
void set_camera(ilG_camera *cam);
void set_walk_direction(il_vec3 vec);
void add_heightmap(ilA_img *hm, float w, float h, float height);
void add_ball(il_positionable *pos);
void update();
void debug_draw();

]]

math.randomseed(os.time())

local skybox = {
    'demos/bouncing-lights/north.png',
    'demos/bouncing-lights/south.png',
    'demos/bouncing-lights/up.png',
    'demos/bouncing-lights/down.png',
    'demos/bouncing-lights/west.png',
    'demos/bouncing-lights/east.png',
}
local c, w, root = helper.context { name="Bouncing Lights Demo",
                                    skybox=skybox,
                                    geom=true,
                                    lights=true,
                                    transparency=true,
                                    gui=true,
                                    output=true,
                                    hints = {hdr=1}}
ffi.C.set_world(w)

local ht = texture()
ht:setContext(c)
local hmt = image.loadfile "demos/bouncing-lights/arena-heightmap.png"
ffi.C.add_heightmap(hmt, 128, 128, 50)
ht:fromimage("height0", hmt)
ht:fromimage("normal0", hmt:height_to_normal())
ht:fromfile("color0", "demos/bouncing-lights/terrain.png")
local hm = positionable()
w:add(hm)
hm.drawable = heightmap(c, 100, 100)
hm.material = heightmap.defaultShader(c)
hm.texture = ht
hm.position = vector3(0, 0, 0).ptr
hm.size = vector3(128, 64, 128).ptr
hm:track(c)

ffi.cdef [[
void glUniform4f(int location, float v0, float v1, float v2, float v3);
]]
local function customdatafunc(self, uniform, user)
    ffi.C.glUniform4f(uniform, 0.0, 0.0, 1.0, 0.25)
end

local glow = material()
glow:vertex(io.open("demos/bouncing-lights/glow.vert","r"):read "*a")
glow:fragment(io.open("demos/bouncing-lights/glow.frag", "r"):read "*a")
glow:mtlname "Glow material"
glow:arrayAttrib("position", "in_Position")
glow:matrix("MVP", "mvp")
glow:customConstant(customdatafunc, "col")
glow:link(c)

_G.num_lights = 0
--event.setPacker(event.registry, "physics.tick", event.nilPacker)
--event.timer(event.registry, "physics.tick", 1/60)
local sphere = drawnmesh("demos/bouncing-lights/sphere.obj")
drawable.setattr(sphere, "istransparent", true)
event.register(event.registry, "input.button", function(reg, name, key, scancode, device, isDown, mods) 
    if key == ' ' and isDown then
        print("Placing lights.")
        local hw, hh = 128, 128
        num_lights = num_lights + 100
        for i = 1, 100 do
            local l = light()
            local pos = vector3(math.random(0,hw-1), math.random(16,48), math.random(0,hh-1))
            --local height = hmt:getPixel(pos.x, pos.z) 
            l.positionable.position = pos.ptr
            l.positionable.size = vector3(.25, .25, .25).ptr
            l.radius = math.random(1, 15)
            l.color = vector3(math.random(0,1), math.random(0,1), math.random(0,1)).ptr
            l:add(c)
            ffi.C.add_ball(l.positionable)
            l.positionable.drawable = sphere
            l.positionable.material = glow
            local tex = texture()
            tex:setContext(c)
            l.positionable.texture = tex
            hm.track(l.positionable, c) --ugh
            w:add(l.positionable)
        end
    end
end)
event.register(event.registry, "tick", function() ffi.C.update() end)

camera(c, root)

c:setActive()

--ffi.C.debug_draw()

