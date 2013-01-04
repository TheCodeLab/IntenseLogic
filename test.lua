package.path = package.path..";script/?.lua;script/graphics/?.lua;script/common/?.lua"
local script = require "script"
require "graphics"
local positionable = require "positionable"
local world = require "world"

local w = world();
local c = context();
c.world = w;
w.context = c;
c:setActive();
local box = positionable(w);
box.drawable = drawable.box;
box.material = material.default;
box.texture = texture.default;
box:track(c);

local cam = camera(positionable(w));
c.camera = cam;

