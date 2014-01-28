require "strict"

local texture       = require "graphics.texture"
local material      = require "graphics.material"
local positionable  = require "common.positionable"
local vector3       = require "math.vector3"
local vector4       = require "math.vector4"
local light         = require "graphics.light"
local image         = require "asset.image"
local heightmap     = require "graphics.heightmap"
local helper        = require "demos.helper"
local event         = require "common.event"
local ffi           = require "ffi"
local drawable      = require "graphics.drawable"
local camera        = require "demos.bouncing-lights.camera"
local drawnmesh     = require "graphics.mesh"
local base          = require "common.base"
local input         = require "input.input"
local array         = require 'common.vector'

ffi.cdef [[

void set_world(il_world *w);
void set_camera(ilG_camera *cam);
void set_walk_direction(il_vec3 vec);
void add_heightmap(ilA_img *hm, float w, float h, float height);
void add_ball(il_positionable *pos);
void update(int debug);
void debug_draw();
void custom_data_func(struct ilG_material *self, il_positionable *pos, GLuint loc, void *user);
ilG_stage *init_stage(ilG_context *context);

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
local c, w, root, pipe
c, w, root, pipe = helper.context { name="Bouncing Lights Demo",
                                    skybox=skybox,
                                    geom=true,
                                    lights=true,
                                    transparency=true,
                                    gui=true,
                                    output=true,
                                    hints = {hdr=1,debug_context=1} }
local pipe2 = {pipe[1], pipe[2], pipe[3], pipe[4], modules.bouncinglights.init_stage(c), pipe[5], pipe[6]}
--c:addStage(pipe2[5], 5)
modules.bouncinglights.set_world(w)

local hmt = image.loadfile "demos/bouncing-lights/arena-heightmap.png"
modules.bouncinglights.add_heightmap(hmt, 128, 128, 50)
local glow = material()
local ht = texture()
ht:setContext(c)
ht:fromimage("height0", hmt)
ht:fromimage("normal0", hmt:height_to_normal())
ht:fromfile("color0", "demos/bouncing-lights/terrain.png")
local hm = positionable()
w:add(hm)
hm.drawable = heightmap(c, 100, 100)
hm.material = heightmap.defaultShader(c)
hm.texture = ht
hm.position = vector3(0, 0, 0).ptr
hm.size = vector3(128, 50, 128).ptr
hm:track(c)

glow:vertex(io.open("demos/bouncing-lights/glow.vert","r"):read "*a")
glow:fragment(io.open("demos/bouncing-lights/glow.frag", "r"):read "*a")
glow:mtlname "Glow material"
glow:arrayAttrib("position", "in_Position")
glow:matrix("MVP", "mvp")
glow:posFunc(modules.bouncinglights.custom_data_func, "col")
glow:link(c)

_G.num_lights = 0
--event.setPacker(event.registry, "physics.tick", event.nilPacker)
--event.timer(event.registry, "physics.tick", 1/60)
local sphere = drawnmesh("demos/bouncing-lights/sphere.obj")
--drawable.setattr(sphere, "istransparent", true)
local debugRender = false --true
event.register(input.button, function(key, scancode, device, isDown, mods)
    if key < 512 then 
        key = string.char(key)
    end
    if key == '1' and isDown then
        glow:vertex(io.open("demos/bouncing-lights/glow.vert","r"):read "*a")
        glow:fragment(io.open("demos/bouncing-lights/glow.frag", "r"):read "*a")
        glow:link(c)
    end
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
            local col = vector3(math.random(0.1,1), math.random(0,1), math.random(0,1)).normal
            l.color = col.ptr
            l:add(c)
            local m = 1
            base.set(l.positionable, 'color', array(col.x*m, col.y*m, col.z*m, 1.0))
            modules.bouncinglights.add_ball(l.positionable)
            l.positionable.drawable = sphere
            l.positionable.material = glow
            local tex = texture()
            tex:setContext(c)
            l.positionable.texture = tex
            positionable.track(l.positionable, c) --ugh
            w:add(l.positionable)
        end
    end
    if key == 'B' and isDown then
        print("Toggling debug rendering")
        debugRender = not debugRender
        c:hint("debug_render", debugRender and 1 or 0)
        c:clearStages()
        local p = debugRender and pipe2 or pipe
        local t = {}
        for _, v in pairs(p) do
            t[#t+1] = ffi.string(v.name)
            c:addStage(v, -1)
        end
        print("Stages loaded: "..table.concat(t, " -> "))
    end
end)
local tick = event(1/20, "bouncinglights.tick")
event.register(tick, function() modules.bouncinglights.update(debugRender and 1 or 0) end)

camera(c, root, tick)

c:start()

--modules.bouncinglights.debug_draw()

