return {
    toUnit = function(str)
        if type(str) == "number" then return str end
        local attrib
        if str == "none" then
            attrib = 0
        elseif str:sub(1, 5) == "color" then
            attrib = 1 + tonumber(str:sub(6, -1))
        elseif str:sub(1, 6) == "normal" then
            attrib = 32 + tonumber(str:sub(7, -1))
        elseif str:sub(1, 4) == "bump" then
            attrib = 64 + tonumber(str:sub(5, -1))
        elseif str:sub(1, 7) == "shadow" then
            attrib = 96 + tonumber(str:sub(8, -1))
        elseif str:sub(1, 7) == "height" then
            attrib = 128 + tonumber(str:sub(8, -1))
        elseif str == "pagetable" then
            attrib = 160
        elseif str == "virtualtex" then
            attrib = 161
        else
            error("Unknown texture unit")
        end
        return attrib;
    end
}

