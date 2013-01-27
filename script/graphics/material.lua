local ffi = require "ffi";

local ilstring = require "ilstring"

ffi.cdef [[

struct ilG_material* ilG_material_default;

struct ilG_material* ilG_material_new(il_string vertsource, il_string fragsource, 
    const char *name, const char *position, const char *texcoord,
    const char *normal, const char *mvp, const char **unitlocs, 
    unsigned long *unittypes);

]]

local material = {};

local function wrap(ptr)
    local obj = {}
    obj.ptr = ptr;
    setmetatable(obj, {__index=material})
    return obj;
end
material.wrap = wrap;

material.default = wrap(ffi.C.ilG_material_default);

-- touch this function and I assume no responsibility for segfaults
function material.create(vert, frag, name, position, texcoord, normal, mvp, unitlocs, unittypes)
    assert(type(unitlocs) == "table" and type(unittypes) == "table")
    assert(type(mvp) == "string" and type(vert) == "string" and type(frag) == "string");

    print "-- unitlocs --"
    local const_char_ptr = ffi.typeof("const char*[$]", #unitlocs + 1);
    local unitlocs_arr = ffi.new(const_char_ptr);
    for i = 1, #unitlocs do
        unitlocs_arr[i-1] = unitlocs[i]
        print(unitlocs_arr[i-1])
    end
    unitlocs_arr[#unitlocs] = nil;
    local unitlocs_arr_ptr = ffi.new("const char**", unitlocs_arr);
    print(unitlocs_arr[#unitlocs]);
    print(unitlocs_arr, unitlocs_arr_ptr);

    print "-- unittypes --"
    local unsigned_long_ptr = ffi.typeof("unsigned long[$]", #unittypes);
    unittypes_arr = ffi.new(unsigned_long_ptr);
    for i = 1, #unittypes do
        unittypes_arr[i-1] = unittypes[i];
        print(unittypes_arr[i-1])
    end
    local unittypes_arr_ptr = ffi.new("unsigned long*", unittypes_arr);
    print(unittypes_arr, unittypes_arr_ptr);

    name = ffi.cast("const char*", name);
    position = ffi.cast("const char*", position);
    normal = ffi.cast("const char*", normal);
    mvp = ffi.cast("const char*", mvp);
    vert = ilstring(vert)
    frag = ilstring(frag)

    print "-- args --"
    print(vert, frag, name, position, texcoord, normal, mvp, unitlocs_arr_ptr, unittypes_arr_ptr)
    local res = ffi.C.ilG_material_new(vert, frag, name, position, texcoord, normal, mvp, unitlocs_arr_ptr, unittypes_arr_ptr)
    print "hi"
    return material.wrap(res);
end

setmetatable(material, {__call = function(self, ...) return material.create(...) end})

return material;

