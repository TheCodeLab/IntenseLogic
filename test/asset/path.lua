local path = require "asset.path"

local test1 = path "/some/made/up/path/to/nowhere.ext"
local test2 = path "some/relative/made/up/path.ext"

print(test1, test2)

