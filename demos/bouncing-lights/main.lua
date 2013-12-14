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

ffi.cdef [[

void set_world(il_world *w);
void set_camera(ilG_camera *cam);
void set_walk_direction(il_vec3 vec);
void add_heightmap(ilA_img *hm, float w, float h, float height);
void add_ball(il_positionable *pos);
void update();
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
local debug_draw = true
if not debug_draw then 
    c, w, root, pipe = helper.context { name="Bouncing Lights Demo",
                                        skybox=skybox,
                                        geom=true,
                                        lights=true,
                                        transparency=true,
                                        gui=true,
                                        output=true,
                                        hints = {hdr=1} }
else
    c, w, root, pipe = helper.context { name="Bouncing Lights Demo - Bullet Debug",
                                        --gui=true,
                                        --output=true,
                                        hints = {hdr=0, major=2, minor=1, use_default_fb=1} }
    c:addStage(modules.bouncinglights.init_stage(c), 0)
end
modules.bouncinglights.set_world(w)

local hmt = image.loadfile "demos/bouncing-lights/arena-heightmap.png"
modules.bouncinglights.add_heightmap(hmt, 128, 128, 50)
local glow = material()
if not debug_draw then
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
    hm.size = vector3(128, 64, 128).ptr
    hm:track(c)

    glow:vertex(io.open("demos/bouncing-lights/glow.vert","r"):read "*a")
    glow:fragment(io.open("demos/bouncing-lights/glow.frag", "r"):read "*a")
    glow:mtlname "Glow material"
    glow:arrayAttrib("position", "in_Position")
    glow:matrix("MVP", "mvp")
    glow:posFunc(modules.bouncinglights.custom_data_func, "col")
    glow:link(c)
end

_G.num_lights = 0
--event.setPacker(event.registry, "physics.tick", event.nilPacker)
--event.timer(event.registry, "physics.tick", 1/60)
local sphere = drawnmesh("demos/bouncing-lights/sphere.obj")
--drawable.setattr(sphere, "istransparent", true)
event.register(event.registry, "input.button", function(reg, name, key, scancode, device, isDown, mods)
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
            local col = vector3(math.random(0,1), math.random(0,1), math.random(0,1)).normal
            l.color = col.ptr
            l:add(c)
            local m = 1
            base.set(l.positionable, "color", ffi.new("float[4]", col.x*m, col.y*m, col.z*m, 1.0), bit.bor(modules.bouncinglights.IL_ARRAY_BIT, modules.bouncinglights.IL_FLOAT), 4)
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
end)
event.register(event.registry, "tick", function() modules.bouncinglights.update() end)

camera(c, root)

c:setActive()

--modules.bouncinglights.debug_draw()

