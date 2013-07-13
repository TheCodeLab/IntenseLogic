local ffi = require "ffi"

ffi.cdef [[

enum ilA_mesh_primitive {
    ILA_MESH_POINTS,
    ILA_MESH_LINES, 
    ILA_MESH_LINE_STRIP, 
    ILA_MESH_LINE_LOOP, 
    ILA_MESH_LINE_STRIP_ADJACENCY, 
    ILA_MESH_LINES_ADJACENCY, 
    ILA_MESH_TRIANGLES, 
    ILA_MESH_TRIANGLE_STRIP, 
    ILA_MESH_TRIANGLE_FAN,
    ILA_MESH_TRIANGLE_STRIP_ADJACENCY, 
    ILA_MESH_TRIANGLES_ADJACENCY,
    ILA_MESH_PATCHES
};

typedef struct ilA_mesh {
    enum ilA_mesh_primitive mode;
    size_t num_vertices, texcoord_size;
    float (*position)[4];
    float (*texcoord)[4];
    float (*normal)[4];
    unsigned char (*ambient)[4];
    unsigned char (*diffuse)[4];
    unsigned char (*specular)[4];
} ilA_mesh;

ilA_mesh *ilA_mesh_load(il_base *file, const ilA_file *iface);
ilA_mesh *ilA_mesh_loadfile(const char *path);
ilA_mesh *ilA_mesh_loadmem(const char *filename, const void *data, size_t length);
void ilA_mesh_free(ilA_mesh *self);
ilA_mesh *ilA_mesh_debugLines(ilA_mesh *self, float f);

]]

local mesh = {}

local function set_gc(m)
    ffi.gc(m, modules.asset.ilA_mesh_free)
    return m
end

mesh.load = function(f) return set_gc(modules.asset.ilA_mesh_load(f, nil)) end
mesh.loadfile = function(f) return set_gc(modules.asset.ilA_mesh_loadfile(f)) end
mesh.loadstr = function(f, s) return set_gc(modules.asset.ilA_mesh_loadmem(f, s, #s)) end
mesh.debugLines = function(self, f) return set_gc(modules.asset.ilA_mesh_debugLines(self, f or 1)) end

ffi.metatype("ilA_mesh", {__index=mesh})

return mesh

