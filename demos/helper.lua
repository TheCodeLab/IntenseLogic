local file          = require 'asset.file'
local input         = require 'input.input'
local quaternion    = require 'math.quaternion'
local vector3       = require 'math.vector3'
local frame         = require 'graphics.gui.frame'
local text          = require 'graphics.gui.text'
local event         = require 'common.event'
local camera        = require 'graphics.camera'
local matrix        = require 'math.matrix'
local context       = require 'graphics.context'
local world         = require 'common.world'
local geometrypass  = require 'graphics.geometrypass'
local lightpass     = require 'graphics.lightpass'
local guipass       = require 'graphics.guipass'
local stage         = require 'graphics.stage'
local outpass       = require 'graphics.outpass'
local texture       = require 'graphics.texture'
local image         = require 'asset.image'
local skyboxpass    = require 'graphics.skyboxpass'

local helper = {}

function helper.context(args)
    local w = world()
    local c = context()
    c:resize(800, 600, args.name or "IntenseLogic Demo")
    c.world = w
    w.context = c
    if args.skybox then -- skybox pass
        local skybox = texture()
        skybox:setContext(c)
        local test_img = image.loadfile(args.skybox)
        skybox:cubemap("color0", {test_img, test_img, test_img, test_img, test_img, test_img})
        local s = stage()
        s.context = c
        skyboxpass(s, skybox)
        c:addStage(s, -1)
    end
    if args.geom then -- geometry pass
        local s = stage()
        s.context = c
        geometrypass(s)
        c:addStage(s, -1)
    end
    if args.lights then -- light pass
        local s = lightpass(c)
        c:addStage(s, -1)
    end
    local root
    if args.gui then-- gui pass
        local s = guipass(c)
        root = frame()
        s:setRoot(root)
        c:addStage(s, -1)
    end
    if args.output then -- output pass
        c:addStage(outpass(c), -1)
    end

    c:setActive()
    return c, w, root
end

function helper.camera(ctx, root)
    local cam = camera()
    ctx.camera = cam
    cam.projection_matrix = matrix.perspective(75, 4/3, 2, 2000).ptr
    cam.positionable.position = vector3(0, 0, 0).ptr
    cam.sensitivity = .01
    cam.movespeed = vector3(1,1,1).ptr

    local first_mouse = true
    local mousemove = function(reg, name, xabs, yabs, x, y)
        if first_mouse then first_mouse = false return end
        if not input.get "mouse left" then return end
        local yaw = quaternion(vector3(0, 1, 0), x * cam.sensitivity)
        local pitch = quaternion(vector3(1, 0, 0), y * cam.sensitivity)
        local rot = quaternion.wrap(cam.positionable.rotation) * yaw * pitch
        cam.positionable.rotation = rot.ptr
    end

    local georgia = file.load "georgia.ttf"
    local camera_pos_label = frame()
    camera_pos_label.context = ctx
    camera_pos_label:setPosition(5,-19, 0, 1)
    root:addChild(camera_pos_label)
    local render_pos = function(pos)
        local label = text(ctx, "en", "ltr", "latin", georgia, 14, tostring(pos))
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
        local r = get("Q") - get("E")
        local v = vector3(x,y,z) * vector3.wrap(cam.movespeed)
        v = v * quaternion.wrap(cam.positionable.rotation)
        cam.positionable.position = (vector3.wrap(cam.positionable.position) + v).ptr
        local bank = quaternion(vector3(0, 0, 1), r * cam.sensitivity * 4)
        cam.positionable.rotation = (quaternion.wrap(cam.positionable.rotation) * bank).ptr
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

return helper

