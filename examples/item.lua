--- Example of the IntenseLogic Object system
-- A simple text RPG item type.
-- @author tiffany
-- @see itemTest.lua

local base = require "base"

return base.type "item" {
    constructor = function(self)
        self.name = "Unnamed"
        self.damage_value = 0
    end;
    __call = function(self,name,damage)
        local inst = self:create()
        inst.name = name
        print(inst.name)
        inst.damage_value = damage
        return inst
    end;
    describeVigor = function(v)
        if v <= 10 then return "" end
        if v <= 100 then return "with the strength of a Dwarven man!" end
        if v <= 200 then return "with the strength of a Golem!" end
        if v <= 1000 then return "with the power of 5 men!" end
        if v <= 1500 then return "so hard the earth cracked beneath you!" end
        return "with the power of a God"
    end;
    use = function(self, vigor)
        assert(self.name)
        print("H"..string.rep("N", math.log(vigor)).."G! Used item "..self.name.." "..self:describeVigor(vigor))
        self:damage(vigor)
    end;
    destroy = function(self)
        print("Threw away "..self.name)
        if self.damage_value > 100 then
            print("In such pristine condition, too! What a waste!")
        end
    end;
    damage = function(self, amount)
        if not self.damage_value then
            self.damage_value = 1000
        end
        self.damage_value = self.damage_value - amount
        print(self.name.." took "..amount.." damage points.")
        if self.damage_value < 0 then
            print(self.name.." broke.")
        end
    end;
}

