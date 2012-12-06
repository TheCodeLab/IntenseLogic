#include <GL/glew.h>

typedef float ilG_obj_vertex[4];

typedef float ilG_obj_texcoord[3];

typedef float ilG_obj_normal[3];

typedef float ilG_obj_color[3];

typedef struct ilG_obj_face {
  struct ilG_obj_face *next;
  ilG_obj_vertex vertices[4];
  ilG_obj_texcoord texcoords[4];
  ilG_obj_normal normals[4];
  int vertices_index[4], texcoords_index[4], normals_index[4];
  int num;
} ilG_obj_face;

typedef struct ilG_obj_mtl {
  const char *name;
  ilG_obj_color ambient, diffuse, specular;
  float specular_co, transparency;
  int illumination;
} ilG_obj_mtl;

typedef struct ilG_obj_mesh {
  struct ilG_obj_mesh *next;
  ilG_obj_face *first_face;
  ilG_obj_mtl mtl;
  const char *name, *group;
  int smoothing;
} ilG_obj_mesh;

typedef struct ilG_obj_file {
  ilG_obj_mesh * first_mesh;
  const char *name;
} ilG_obj_file;

typedef const char* (*ilG_obj_reader)(const char *name);

const char* ilG_obj_stdio_reader(const char *filename);

ilG_obj_file ilG_obj_readstring(const char * data, ilG_obj_reader inc, const char *filename);

ilG_obj_file ilG_obj_readfile(const char *filename);

enum ilG_obj_vertextype {
  OBJ_NOVERTICES = 0,
  OBJ_X = 1,
  OBJ_Y = 2,
  OBJ_Z = 4,
  OBJ_W = 8,
  OBJ_XYZ = OBJ_X|OBJ_Y|OBJ_Z,
  OBJ_XYZW = OBJ_XYZ|OBJ_W
};

enum ilG_obj_texcoordtype {
  OBJ_NOTEXCOORDS = 0,
  OBJ_U = 1,
  OBJ_V = 2,
  // OBJ_W (already defined)
  OBJ_UV = OBJ_U|OBJ_V,
  OBJ_UVW = OBJ_UV|OBJ_W
};

enum ilG_obj_normaltype {
  OBJ_NONORMALS = 0,
  OBJ_NORMALS = 1
};

enum ilG_obj_facetype {
  OBJ_TRIANGLES,
  OBJ_QUADS
};

enum ilG_obj_vbolayout {
  OBJ_INTERLEAVED,
  OBJ_TIGHT
};

GLfloat *ilG_obj_to_vbo(ilG_obj_mesh *mesh, enum ilG_obj_vertextype vertex, 
  enum ilG_obj_texcoordtype texcoord, enum ilG_obj_normaltype normal, 
  enum ilG_obj_facetype face, enum ilG_obj_vbolayout layout, size_t *size);

GLuint ilG_obj_to_gl(ilG_obj_mesh *mesh, GLint *count);

void ilG_obj_bindmtl(ilG_obj_mtl mtl, GLuint program, const char *ambient, 
  const char *diffuse, const char *specular, const char *specular_co, 
  const char *transparency);

void ilG_obj_bindmtl_defaults(ilG_obj_mtl mtl, GLuint program);
  