--- Test of item.lua
-- @author tiffany
-- @see item.lua

package.path = package.path..";examples/?.lua;script/?.lua;script/graphics/?.lua;script/common/?.lua"

local item = require "item"

Axe = item("Ogre's Axe", 5000)
Axe:use(100)
Axe:use(1000)
Axe:destroy()

Longsword = item("German Longsword", 2000)
Longsword:use(10)
Longsword:use(200)
Longsword:damage(1800)

