local camera        = require "graphics.camera"
local matrix        = require "math.matrix"
local vector3       = require "math.vector3"
local quaternion    = require "math.quaternion"
local file          = require "asset.file"
local frame         = require "graphics.gui.frame"
local text          = require "graphics.gui.text"
local event         = require "common.event"
local input         = require "input.input"

return function(ctx, root)
    local cam = camera()
    ctx.camera = cam
    cam.projection_matrix = matrix.perspective(75, 4/3, 2, 2000).ptr
    cam.positionable.position = vector3(64, 16, 64).ptr
    cam.positionable.rotation = quaternion(0, 0, 1, math.pi).ptr
    cam.sensitivity = .01
    cam.movespeed = vector3(1,1,1).ptr

    local yaw = 0
    local pitch = 0

    local first_mouse = true
    local mousemove = function(reg, name, xabs, yabs, x, y)
        if first_mouse then first_mouse = false return end
        if not input.get "mouse left" then return end
        yaw = yaw + x * cam.sensitivity
        pitch = pitch - y * cam.sensitivity
        yaw = yaw % (math.pi*2)
        pitch = math.max(-math.pi/2, math.min(math.pi/2, pitch))
        cam.positionable.rotation = (quaternion(0, 1, 0, -yaw) * quaternion(1, 0, 0, pitch) * quaternion(0, 0, 1, math.pi)).ptr
    end

    local georgia = file.load "georgia.ttf"
    local camera_pos_label = frame()
    camera_pos_label.context = ctx
    camera_pos_label:setPosition(5,-19, 0, 1)
    root:addChild(camera_pos_label)
    local render_pos = function(pos)
        local label = text(ctx, "en", "ltr", "latin", georgia, 14, tostring(pos)..string.format(" (%d %d)", pitch * 180 / math.pi, yaw * 180 / math.pi))
        camera_pos_label:label(label, {1,1,1,1}, "left middle")
    end

    local fps_label = frame()
    fps_label.context = ctx
    fps_label:setPosition(5, 5, 0, 0)
    root:addChild(fps_label)
    local render_fps = function(f)
        local label = text(ctx, "en", "ltr", "latin", georgia, 14, string.format("FPS: %.1f", tonumber(f)))
        fps_label:label(label, {1,1,1,1}, "left middle")
    end

    local tick = function(reg, name)
        local get = function(k)
            local b, _ = input.get(k)
            return b and 1 or 0
        end
        local x = get("A") - get("D")
        local z = get("S") - get("W")
        local y = get("F") - get("R")
        local v = vector3(x,y,z) * vector3.wrap(cam.movespeed)
        v = v * quaternion.wrap(cam.positionable.rotation)
        cam.positionable.position = (vector3.wrap(cam.positionable.position) + v).ptr
        render_pos(vector3(cam.positionable.position))
        render_fps(1/ctx:averageFrametime())
        cam.projection_matrix = matrix.perspective(75, ctx.width/ctx.height, 2, 2000).ptr
    end

    local close = function(reg, name)
        event.event(event.registry, "shutdown")
    end

    event.register(event.registry, "tick", tick)
    event.register(event.registry, "input.mousemove", mousemove)
    event.register(event.registry, "graphics.close", close)
end

