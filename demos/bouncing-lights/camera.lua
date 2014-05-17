local camera        = require "graphics.camera"
local matrix        = require "math.matrix"
local vector3       = require "math.vector3"
local quaternion    = require "math.quaternion"
local file          = require "asset.file"
local event         = require "common.event"
local input         = require "input.input"
local ffi           = require "ffi"
local positionable  = require "common.positionable"
local renderer      = require "graphics.renderer"

return function(ctx, w, tick)
    local cam = camera()
    ctx.camera = cam
    cam.projection_matrix = matrix.perspective(75, 4/3, .5, 512).ptr
    cam.positionable = positionable(w)
    cam.positionable.position = vector3(64, 32, 64).ptr
    cam.positionable.rotation = quaternion(vector3(0, 0, 1), math.pi).ptr
    cam.sensitivity = .01
    cam.movespeed = vector3(1/3,1/3,1/3).ptr
    modules.bouncinglights.set_camera(cam)

    local yaw = 0
    local pitch = 0

    local mousemove = function(xabs, yabs, x, y)
        if not input.get "mouse left" then return end
        yaw = yaw + x * cam.sensitivity
        pitch = pitch - y * cam.sensitivity
        yaw = yaw % (math.pi*2)
        pitch = math.max(-math.pi/2, math.min(math.pi/2, pitch))
        cam.positionable.rotation = (quaternion(vector3(0, 1, 0), -yaw) * quaternion(vector3(1, 0, 0), pitch) * quaternion(vector3(0, 0, 1), math.pi)).ptr
    end

    local ontick = function()
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
        cam.projection_matrix = matrix.perspective(75, ctx.width/ctx.height, 2, 2000).ptr
    end

    local on_after_close = event()
    local after_close = function(hnd)
        event.destroy(tick)
        ctx:stop()
        ctx:destroy()
        event.fireAsync(event.shutdown)
    end
    local close = function(hnd)
        event.fireAsync(on_after_close)
    end

    event.register(tick, ontick)
    event.register(input.mousemove, mousemove)
    event.register(ctx.close, close)
    event.register(on_after_close, after_close)
end

