local camera        = require "graphics.camera"
local matrix        = require "math.matrix"
local vector3       = require "math.vector3"
local quaternion    = require "math.quaternion"
local file          = require "asset.file"
local frame         = require "graphics.gui.frame"
local text          = require "graphics.gui.text"
local event         = require "common.event"
local input         = require "input.input"
local ffi           = require "ffi"

return function(ctx, root)
    local cam = camera()
    ctx.camera = cam
    cam.projection_matrix = matrix.perspective(75, 4/3, .5, 512).ptr
    cam.positionable.position = vector3(64, 32, 64).ptr
    cam.positionable.rotation = quaternion(vector3(0, 0, 1), math.pi).ptr
    cam.sensitivity = .01
    cam.movespeed = vector3(1/3,1/3,1/3).ptr
    modules.bouncinglights.set_camera(cam)

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
        cam.positionable.rotation = (quaternion(vector3(0, 1, 0), -yaw) * quaternion(vector3(1, 0, 0), pitch) * quaternion(vector3(0, 0, 1), math.pi)).ptr
    end

    local render_pos, render_numlights, render_fps
    if root then
        local georgia = file.load "georgia.ttf"
        local camera_pos_label = frame()
        camera_pos_label.context = ctx
        camera_pos_label:setPosition(5,-19, 0, 1)
        root:addChild(camera_pos_label)
        render_pos = function(pos)
            local label = text(ctx, "en", "ltr", "latin", georgia, 14, tostring(pos)..string.format(" (%d %d)", pitch * 180 / math.pi, yaw * 180 / math.pi))
            camera_pos_label:label(label, {1,1,1,1}, "left middle")
        end

        local fps_label = frame()
        fps_label.context = ctx
        fps_label:setPosition(5, 5, 0, 0)
        root:addChild(fps_label)
        render_fps = function(f)
            local label = text(ctx, "en", "ltr", "latin", georgia, 14, string.format("FPS: %.1f", tonumber(f)))
            fps_label:label(label, {1,1,1,1}, "left middle")
        end

        local lights_label = frame()
        lights_label.context = ctx
        lights_label:setPosition(5, 20, 0, 0)
        root:addChild(lights_label)
        render_numlights = function()
            local label = text(ctx, "en", "ltr", "latin", georgia, 14, string.format("Lights: %d", _G.num_lights))
            lights_label:label(label, {1,1,1,1}, "left middle")
        end

        do 
            local controls_box = frame()
            controls_box.context = ctx
            controls_box:setPosition(-205, 5, 1, 0)
            controls_box:setSize(200, 0, 0, 1)
            root:addChild(controls_box)
            local q_label = frame()
            q_label.context = ctx
            q_label:setPosition(0, 0, 0, 0)
            controls_box:addChild(q_label)
            local label = text(ctx, "en", "ltr", "latin", georgia, 14, "Press '?' for controls.")
            q_label:label(label, {1,1,1,1}, "left middle")
            do
                local hint_box = frame()
                hint_box.context = ctx
                hint_box:setPosition(0, 20, 0, 0)
                hint_box:setSize(0, -20, 1, 1)
                local msgs = {
                    "1: Reload shaders",
                    "Space: Add 100 lights",
                    "B: Toggle debug rendering"
                }
                for i, v in ipairs(msgs) do
                    local f = frame()
                    f.context = ctx
                    f:setPosition(-200, i * 20 - 20, 1, 0)
                    hint_box:addChild(f)
                    local label = text(ctx, "en", "ltr", "latin", georgia, 14, v)
                    f:label(label, {1,1,1,1}, "left middle")
                end
                local isShown = false
                event.register(event.registry, "input.button", function(reg, name, key, scancode, device, isDown, mods)
                    if isDown and key == '/' then
                        isShown = not isShown
                        if isShown then
                            controls_box:addChild(hint_box)
                        else
                            hint_box:pop()
                        end
                    end
                end)
            end
        end
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
        local old = vector3.wrap(cam.positionable.position)
        v = v * quaternion.wrap(cam.positionable.rotation) 
        modules.bouncinglights.set_walk_direction(v.ptr)
        if root then
            render_pos(vector3(cam.positionable.position))
            local avg = ctx:averageFrametime()
            if avg == 0 then
                render_fps(0)
            else
                render_fps(1/avg)
            end
            render_numlights()
        end
        cam.projection_matrix = matrix.perspective(75, ctx.width/ctx.height, 2, 2000).ptr
    end

    local close = function(reg, name)
        event.event(event.registry, "shutdown")
    end

    event.register(event.registry, "tick", tick)
    event.register(event.registry, "input.mousemove", mousemove)
    event.register(event.registry, "graphics.close", close)
end

