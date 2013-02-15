#define _POSIX_SOURCE
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "obj.h"
#include <GL/glew.h>
#include <stdio.h>

enum ilG_obj_line_type {
  OBJ_ERROR,
  OBJ_COMMENT,
  OBJ_VERTEX,
  OBJ_TEXCOORD,
  OBJ_NORMAL,
  OBJ_FACE,
  OBJ_MTLLIB,
  OBJ_USEMTL,
  OBJ_ONAME,
  OBJ_GNAME,
  OBJ_SMOOTHING
};

struct ilG_obj_line {
  struct ilG_obj_line *next;
  enum ilG_obj_line_type type;
  union {
    ilG_obj_vertex vertex;
    ilG_obj_texcoord texcoord;
    ilG_obj_normal normal;
    ilG_obj_face face;
    const char *mtllib, *usemtl, *oname, *gname, *smoothing, *error;
  } line;
};

struct ilG_obj_ctx {
  struct ilG_obj_line *first;
  int line;
};

static struct cache_entry {
  struct ilG_obj_line **data;
  size_t length, capacity;
} ilG_obj_lookup_cache[11];

static void *resize(void *ptr, size_t oldsize, size_t newsize)
{
  void *new = malloc(newsize);
  if (ptr)
    memcpy(new, ptr, oldsize);
  free(ptr);
  return new;
}

static void cache_append(struct ilG_obj_line* l) {
  struct cache_entry *e = &ilG_obj_lookup_cache[l->type];
  if (e->length >= e->capacity) {
    size_t size = e->capacity * sizeof(struct ilG_obj_line*);
    if (size == 0)
      size = 50 * sizeof(struct ilG_obj_line*); // start us off with a large cache
    e->data = resize(e->data, size, size*2);
    e->capacity = size*2 / sizeof(struct ilG_obj_line*);
  }
  e->data[e->length] = l;
  e->length++;
}

static struct ilG_obj_line* ilG_obj_lookup(const struct ilG_obj_ctx* ctx, enum ilG_obj_line_type t, int i)
{
    (void)ctx;
  if ((int)ilG_obj_lookup_cache[t].length >= i && i > 0)
    return ilG_obj_lookup_cache[t].data[i-1];
  if (i < 0)
    return ilG_obj_lookup_cache[t].data[(int)ilG_obj_lookup_cache[t].length + i];
  fprintf(stderr, "Invalid index %i (max %zu)\n", i, ilG_obj_lookup_cache[t].length);
  return NULL;
}

static struct ilG_obj_line *ilG_obj_readline(const struct ilG_obj_ctx* ctx, const char *line)
{
  struct ilG_obj_line *res = calloc(1, sizeof(struct ilG_obj_line));
  int line_len = strlen(line);
  {
    int len = 0;
    sscanf(line, "%*[ \t\n]%n", &len);
    if (len == line_len) {
      res->type = OBJ_COMMENT; // treat empty lines as comments
      return res;
    }
  }
  switch(line[0]) {
    case '#':
      res->type = OBJ_COMMENT;
      break;
    case 'v':
      switch(line[1]) {
        case ' ': 
        case '\t':
        case '\n': {
          res->type = OBJ_VERTEX;
          float *p = &res->line.vertex[0];
          int len, matched;
          matched = sscanf(line+2, "%f %f %f%n", p, p+1, p+2, &len);
          if (matched < 3) {
            res->type = OBJ_ERROR;
            res->line.error = "Too few components in vertex";
            break;
          }
          matched += sscanf(line+2, "%*f %*f %*f %f%n", p+3, &len);
          if (matched < 4)
            res->line.vertex[3] = 1.0;
          if (len + 2 < line_len) {
            res->type = OBJ_ERROR;
            res->line.error = "Junk data after vertex";
          }
          break;
        }
        case 't': {
          res->type = OBJ_TEXCOORD;
          float *p = &res->line.texcoord[0];
          int matched, len;
          matched = sscanf(line+2, "%f %f%n", p, p+1, &len);
          if (matched < 2) {
            res->type = OBJ_ERROR;
            res->line.error = "Too few components in texcoord";
            break;
          }
          matched += sscanf(line+2, "%*f %*f %f%n", p+2, &len);
          if (len + 2 < line_len) {
            res->type = OBJ_ERROR;
            res->line.error = "Junk data after texcoord";
          }
          break;
        }
        case 'n': {
          res->type = OBJ_NORMAL;
          float *p = &res->line.normal[0];
          int len, matched;
          matched = sscanf(line+2, "%f %f %f%n", p, p+1, p+2, &len);
          if (matched < 3) {
            res->type = OBJ_ERROR;
            res->line.error = "Too few components in normal";
          }
          if (len + 2 < line_len) {
            res->type = OBJ_ERROR;
            res->line.error = "Junk data after normal";
          }
          break;
        }
        default: 
          res->type = OBJ_ERROR;
          res->line.error = "Invalid/corrupt line";
          break;
      }
      break;
    case 'f': {
      res->type = OBJ_FACE;
      int *v  = &res->line.face.vertices_index[0],
          *vt = &res->line.face.texcoords_index[0],
          *vn = &res->line.face.normals_index[0];
      int matched, len, i;
      const char *p = line + 2;
      for (i = 0; i < 4; i++) {
        len = 0; // in case it doesn't match
        matched = sscanf(p, "%*[ \t\n]%n", &len);
        if (matched == EOF) break;
        p += len;
        matched = sscanf(p, "%i%n", v+i, &len);
        if (!matched) goto face_error;
        p += len;
        if (*p != '/') continue;
        p++;
        len = 0; // in case it doesn't match
        matched = sscanf(p, "%i%n", vt+i, &len);
        p += len;
        if (*p != '/') continue;
        p++;
        matched = sscanf(p, "%i%n", vn+i, &len);
        if (!matched) goto face_error;
        p += len;
      }
      res->line.face.num = i;
      for (i = 0; i < 4; i++) {
        struct ilG_obj_line *o;
        if (v[i]) {
          o = ilG_obj_lookup(ctx, OBJ_VERTEX, v[i]);
          if (o)
            memcpy(&res->line.face.vertices[i], &o->line.vertex, sizeof(ilG_obj_vertex));
        }
        if (vt[i]) {
          o = ilG_obj_lookup(ctx, OBJ_TEXCOORD, vt[i]);
          if (o)
            memcpy(&res->line.face.texcoords[i], &o->line.texcoord, sizeof(ilG_obj_texcoord));
        }
        if (vn[i]) {
          o = ilG_obj_lookup(ctx, OBJ_NORMAL, vn[i]);
          if (o)
            memcpy(&res->line.face.normals[i], &o->line.normal, sizeof(ilG_obj_normal));
        }
      }
      break;
      face_error:
      res->type = OBJ_ERROR;
      res->line.error = "Invalid face definition";
      break;
    }
    case 'm':
      res->type = OBJ_MTLLIB;
      res->line.mtllib = line + 7;
      break;
    case 'u':
      res->type = OBJ_USEMTL;
      res->line.usemtl = line + 7;
      break;
    case 'o':
      res->type = OBJ_ONAME;
      res->line.oname = line + 2;
      break;
    case 'g':
      res->type = OBJ_GNAME;
      res->line.gname = line + 2;
      break;
    case 's':
      res->type = OBJ_SMOOTHING;
      res->line.smoothing = line + 2;
      break;
    default:
      res->type = OBJ_ERROR;
      res->line.error = "Invalid line";
  }
  return res;
}

static ilG_obj_mesh* create_mesh()
{
  ilG_obj_mesh* mesh = calloc(1, sizeof(ilG_obj_mesh));
  
  mesh->mtl = (ilG_obj_mtl){
    NULL,       // name
    {1, 1, 1},  // ambient
    {0, 0, 0},  // diffuse
    {0, 0, 0},  // specular
    0.0,        // specular coefficient
    1.0,        // transparency
    0           // illumination
  };
  
  return mesh;
}

enum mtl_linetype {
  MTL_ERROR,
  MTL_COMMENT,
  MTL_NEWMTL,
  MTL_AMBIENT,
  MTL_DIFFUSE,
  MTL_SPECULAR,
  MTL_SPECULAR_CO,
  MTL_TRANSPARENCY,
  MTL_ILLUM,
};

struct mtl_file {
  struct mtl_file *next;
  ilG_obj_mtl mtl;
};

struct mtl_line {
  enum mtl_linetype type;
  union {
    const char *error, *newmtl;
    ilG_obj_color color;
    float specular_co, transparency;
    int illum;
  } line;
};

static struct mtl_line mtl_readline(const char *line)
{
  struct mtl_line res;
  memset(&res, 0, sizeof(struct mtl_line));
  int line_len = strlen(line);
  {
    int len = 0;
    sscanf(line, "%*[ \t\n]%n", &len);
    if (len == line_len) {
      res.type = OBJ_COMMENT; // treat empty lines as comments
      return res;
    }
  }
  
  switch (line[0]) {
    case '#':
      res.type = MTL_COMMENT;
      break;
    case 'n':
      res.type = MTL_NEWMTL;
      res.line.newmtl = line + 7;
      break;
    case 'K': {
      float *c = &res.line.color[0];
      int matched, len;
      matched = sscanf(line + 3, "%f %f %f%n", c, c+1, c+2, &len);
      if (matched < 3) {
        res.type = MTL_ERROR;
        res.line.error = "Invalid color definition";
        break;
      }
      switch (line[1]) {
        case 'a': res.type = MTL_AMBIENT; break;
        case 'd': res.type = MTL_DIFFUSE; break;
        case 's': res.type = MTL_SPECULAR; break;
        default:
          res.type = MTL_ERROR;
          res.line.error = "Invalid line";
          break;
      }
      if (len + 3 < line_len) {
        res.type = MTL_ERROR;
        res.line.error = "Junk data after color definition";
      }
      break;
    }
    case 'N': {
      res.type = MTL_SPECULAR_CO;
      if (line[1] != 's') {
        res.type = MTL_ERROR;
        res.line.error = "Invalid line";
        break;
      }
      int matched, len;
      matched = sscanf(line+3, "%f%n", &res.line.specular_co, &len);
      if (matched < 1) {
        res.type = MTL_ERROR;
        res.line.error = "Invalid specular coefficient";
      }
      break;
    }
    case 'T':
      if (line[1] != 'r') {
        res.type = MTL_ERROR;
        res.line.error = "Invalid line";
        break;
      }
    case 'd': {
      res.type = MTL_TRANSPARENCY;
      int matched, len;
      matched = sscanf(line+(line[0]=='d'? 2 : 3), "%f%n", &res.line.transparency, &len);
      if (matched < 1) {
        res.type = MTL_ERROR;
        res.line.error = "Invalid transparency value";
      }
      break;
    }
    case 'i':
      res.type = MTL_ILLUM;
      res.line.illum = atoi(line + 6);
      break;
    default:
      res.type = MTL_ERROR;
      res.line.error = "Invalid line";
  }
  
  return res;
}

static struct mtl_file *mtl_readstring(const char * data, const char *filename)
{
  int             cur_line = 0, 
                  data_len, 
                  line_len;
  const char      *line_ptr = data;
  char            *line_str;
  struct mtl_file *first, 
                  *cur;
  
  memset(ilG_obj_lookup_cache, 0, sizeof(ilG_obj_lookup_cache)); // clear the cache
  first = cur = calloc(1, sizeof(struct mtl_file));
  data_len = strlen(data);
  
  while (line_ptr < data+data_len) {
    line_len = 0;
    while (line_ptr[line_len] != '\n' && line_ptr[line_len] != '\0') 
      line_len++;
    line_str = malloc(line_len + 1);
    memcpy(line_str, line_ptr, line_len);
    line_str[line_len] = 0;
    line_ptr += line_len + 1;
    
    struct mtl_line line = mtl_readline(line_str);
    cur_line++;
  
    switch (line.type) {
      case MTL_ERROR:
        fprintf(stderr, "[%s:%i] %s: %s\n", filename, cur_line, line.line.error, line_str);
        break;
      case MTL_NEWMTL:
        cur->next = calloc(1, sizeof(struct mtl_file));
        cur = cur->next;
        cur->mtl.name = line.line.newmtl;
        break;
      case MTL_AMBIENT:
        memcpy(cur->mtl.ambient, line.line.color, sizeof(ilG_obj_color));
        break;
      case MTL_DIFFUSE:
        memcpy(cur->mtl.diffuse, line.line.color, sizeof(ilG_obj_color));
        break;
      case MTL_SPECULAR:
        memcpy(cur->mtl.specular, line.line.color, sizeof(ilG_obj_color));
        break;
      case MTL_SPECULAR_CO:
        cur->mtl.specular_co = line.line.specular_co;
        break;
      case MTL_TRANSPARENCY:
        cur->mtl.transparency = line.line.transparency;
        break;
      case MTL_ILLUM:
        cur->mtl.illumination = line.line.illum;
        break;
      default:
        break;
    }
  }
  
  return first;
}

ilG_obj_file ilG_obj_readstring(const char * data, ilG_obj_reader inc, const char *filename)
{
  // declare a boatload of variables
  ilG_obj_file        file;
  struct ilG_obj_ctx  ctx;
  int             data_len, 
                  line_len;
  const char      *line_ptr = data, 
                  *line_end,
                  *cur_gname = NULL;
  char            *line_str;
  struct ilG_obj_line *last = NULL;
  struct mtl_file *cur_mtl_file = NULL;
  ilG_obj_mesh        *cur_mesh;
  ilG_obj_face        *cur_face = NULL;
  
  // setup the variables
  ctx = (struct ilG_obj_ctx){NULL,0};
  data_len = strlen(data);
  memset(&file, 0, sizeof(ilG_obj_file));
  file.name = filename;
  cur_mesh = create_mesh();
  file.first_mesh = cur_mesh;
  
  while (line_ptr < data+data_len) {
    line_end = strchr(line_ptr, '\n');
    line_len = line_end - line_ptr;
    if (!line_end) line_end = data+data_len;
    line_str = malloc(line_len + 1);
    memcpy(line_str, line_ptr, line_len);
    line_str[line_len] = 0;
    line_ptr = line_end+1;
    
    struct ilG_obj_line *line = ilG_obj_readline(&ctx, line_str);
    if (last) last->next = line;
    else ctx.first = line;
    last = line;
    ctx.line++;
    
    switch(line->type) {
      case OBJ_ERROR:
        fprintf(stderr, "[%s:%i] %s: %s\n", filename, ctx.line, line->line.error, line_str);
        break;
      case OBJ_VERTEX:
      case OBJ_TEXCOORD:
      case OBJ_NORMAL:
        cache_append(line);
        break;
      case OBJ_MTLLIB: {
        // load the mtl file and parse it
        free(cur_mtl_file);
        cur_mtl_file = mtl_readstring(inc(line->line.mtllib), line->line.mtllib);
        break;
      }
      case OBJ_FACE:
        if (cur_face)
          cur_face->next = &line->line.face;
        else
          cur_mesh->first_face = &line->line.face;
        cur_face = &line->line.face;
        break;
      case OBJ_ONAME:
        if (cur_mesh->name) {
          cur_mesh->next = create_mesh();
          cur_mesh = cur_mesh->next;
        }
        cur_mesh->name = line->line.oname;
        cur_mesh->group = cur_gname;
        break;
      case OBJ_GNAME:
        cur_gname = line->line.gname;
        break;
      case OBJ_SMOOTHING:
        cur_mesh->smoothing = atoi(line->line.smoothing);
        break;
      case OBJ_USEMTL: {
        struct mtl_file * cur = cur_mtl_file;
        do {
          cur = cur->next;
        } while (cur && cur->mtl.name && 
          strcmp(cur->mtl.name, line->line.usemtl) != 0);
        if (cur)
          memcpy(&cur_mesh->mtl, &cur->mtl, sizeof(ilG_obj_mtl));
        break;
      }
      default:
        break;
    }
    free(line_str);
  }

  struct ilG_obj_line *cur = ctx.first;
  last = NULL;
  while (cur) {
    free(last);
    last = cur;
    cur = cur->next;
  }

  unsigned int c;
  for (c = 0; c < 11; c++) {
    ilG_obj_lookup_cache[c].length = 0;
    ilG_obj_lookup_cache[c].capacity = 0;
    free(ilG_obj_lookup_cache[c].data);
  }

  return file;
}

const char* ilG_obj_stdio_reader(const char *filename)
{
  struct stat s;
  stat(filename, &s);
  char *buf = malloc(s.st_size + 1);
  FILE *f = fopen(filename, "r");
  if (!f)
    perror(filename);
  int len = fread(buf, 1, s.st_size, f);
  if (!len)
    perror(filename);
  buf[len] = 0;
  return buf;
}

ilG_obj_file ilG_obj_readfile(const char *filename)
{
  char *data = (char*)ilG_obj_stdio_reader(filename);
  ilG_obj_file f = ilG_obj_readstring(data, ilG_obj_stdio_reader, filename);
  free(data);
  return f;
}

static int copy_vertex(GLfloat *buf, ilG_obj_vertex vert, enum ilG_obj_vertextype filter)
{
    int n = 0;
    if (filter & OBJ_X) {
        buf[0] = vert[0];
        n++;
        buf++;
    }
    if (filter & OBJ_Y) {
        buf[0] = vert[1];
        n++;
        buf++;
    }
    if (filter & OBJ_Z) {
        buf[0] = vert[2];
        n++;
        buf++;
    }
    if (filter & OBJ_W) {
        buf[0] = vert[3];
        n++;
    }
    return n;
}

static int copy_texcoord(GLfloat *buf, ilG_obj_texcoord texcoord, enum ilG_obj_texcoordtype filter)
{
    int n = 0;
    if (filter & OBJ_U) {
        buf[0] = texcoord[0];
        n++;
        buf++;
    }
    if (filter & OBJ_V) {
        buf[0] = texcoord[1];
        n++;
        buf++;
    }
    if (filter & OBJ_W) {
        buf[0] = texcoord[2];
        n++;
    }
    return n;
}

static int copy_normal(GLfloat *buf, ilG_obj_normal normal, enum ilG_obj_normaltype filter)
{
    if (filter) {
        buf[0] = normal[0];
        buf[1] = normal[1];
        buf[2] = normal[2];
        return 3;
    }
    return 0;
}

static int copy_ambient(GLfloat* buf, ilG_obj_color color, enum ilG_obj_materialtype filter)
{
    if (filter & OBJ_AMBIENT) {
        buf[0] = color[0];
        buf[1] = color[1];
        buf[2] = color[2];
        //buf[3] = color[3];
        return 3;
    }
    return 0;
}

static int copy_diffuse(GLfloat* buf, ilG_obj_color color, enum ilG_obj_materialtype filter)
{
    if (filter & OBJ_DIFFUSE) {
        buf[0] = color[0];
        buf[1] = color[1];
        buf[2] = color[2];
        //buf[3] = color[3];
        return 3;
    }
    return 0;
}

static int copy_specular(GLfloat* buf, ilG_obj_color color, float co, enum ilG_obj_materialtype filter)
{
    if (filter & OBJ_SPECULAR) {
        buf[0] = color[0];
        buf[1] = color[1];
        buf[2] = color[2];
        buf[3] = co;
        return 4;
    }
    return 0;
}


static int copy_material(GLfloat *buf, ilG_obj_mtl* mtl, enum ilG_obj_materialtype filter)
{
    int n = 0;
    n += copy_ambient   (buf,       mtl->ambient, filter);
    n += copy_diffuse   (buf + n,   mtl->diffuse, filter);
    n += copy_specular  (buf + n,   mtl->specular, mtl->specular_co, filter);
    return n;
}

GLfloat *ilG_obj_to_vbo(ilG_obj_mesh *mesh, enum ilG_obj_vertextype vertex, 
  enum ilG_obj_texcoordtype texcoord, enum ilG_obj_normaltype normal, 
  enum ilG_obj_materialtype material, enum ilG_obj_facetype face, 
  enum ilG_obj_vbolayout layout, size_t *size, size_t *stride)
{
  int       num_points = 0;
  ilG_obj_face  *cur = mesh->first_face;
  size_t    vertex_size, 
            texcoord_size, 
            normal_size, 
            material_size,
            interleaved_size;
  GLfloat   *buf, 
            *ptr;
  int       per_point = face? 4:3;
  
  vertex_size = sizeof(GLfloat) * (
    ((vertex&OBJ_X) == OBJ_X) + 
    ((vertex&OBJ_Y) == OBJ_Y) + 
    ((vertex&OBJ_Z) == OBJ_Z) +
    ((vertex&OBJ_W) == OBJ_W));
  texcoord_size = sizeof(GLfloat) * (
    ((texcoord&OBJ_U) == OBJ_U) +
    ((texcoord&OBJ_V) == OBJ_V) +
    ((texcoord&OBJ_W) == OBJ_W));
  normal_size = sizeof(ilG_obj_normal) * normal;
  material_size = sizeof(GLfloat) * 3 * (
    ((material&OBJ_AMBIENT) == OBJ_AMBIENT) +
    ((material&OBJ_DIFFUSE) == OBJ_DIFFUSE) +
    ((material&OBJ_SPECULAR) == OBJ_SPECULAR)) +
    sizeof(GLfloat) * ((material&OBJ_SPECULAR) == OBJ_SPECULAR);
  interleaved_size = vertex_size + texcoord_size + normal_size + material_size;
  if (stride) {
      *stride = interleaved_size;
  }
  
  while (cur) {
    if (cur->num == 4 && per_point == 3) {
      num_points += 6;
    } else {
      num_points += cur->num;
    }
    cur = cur->next;
  }
  
  buf = calloc(num_points, interleaved_size);
  ptr = buf;
  
  if (layout) { // tightly packed
    cur = mesh->first_face;
    while (cur) {
      ptr += copy_vertex(ptr, cur->vertices[0], vertex);
      ptr += copy_vertex(ptr, cur->vertices[1], vertex);
      ptr += copy_vertex(ptr, cur->vertices[2], vertex);
      if (face) { // dealing with quads
        ptr += copy_vertex(ptr, cur->vertices[3], vertex);
      } else if (cur->num > 3) { // triangles, and the source isn't a triangle
        ptr += copy_vertex(ptr, cur->vertices[2], vertex);
        ptr += copy_vertex(ptr, cur->vertices[3], vertex);
        ptr += copy_vertex(ptr, cur->vertices[0], vertex);
      }
      cur = cur->next;
    }
    cur = mesh->first_face;
    while (cur) {
      ptr += copy_texcoord(ptr, cur->texcoords[0], texcoord);
      ptr += copy_texcoord(ptr, cur->texcoords[1], texcoord);
      ptr += copy_texcoord(ptr, cur->texcoords[2], texcoord);
      if (face) { 
        ptr += copy_texcoord(ptr, cur->texcoords[3], texcoord);
      } else if (cur->num > 3) { 
        ptr += copy_texcoord(ptr, cur->texcoords[2], texcoord);
        ptr += copy_texcoord(ptr, cur->texcoords[3], texcoord);
        ptr += copy_texcoord(ptr, cur->texcoords[0], texcoord);
      }
      cur = cur->next;
    }
    cur = mesh->first_face;
    while (cur) {
      ptr += copy_normal(ptr, cur->normals[0], normal);
      ptr += copy_normal(ptr, cur->normals[1], normal);
      ptr += copy_normal(ptr, cur->normals[2], normal);
      if (face) { 
        ptr += copy_normal(ptr, cur->normals[3], normal);
      } else if (cur->num > 3) { 
        ptr += copy_normal(ptr, cur->normals[2], normal);
        ptr += copy_normal(ptr, cur->normals[3], normal);
        ptr += copy_normal(ptr, cur->normals[0], normal);
      }
      cur = cur->next;
    }
    cur = mesh->first_face;
    while (cur) {
      ptr += copy_ambient(ptr, mesh->mtl.ambient, material);
      ptr += copy_ambient(ptr, mesh->mtl.ambient, material);
      ptr += copy_ambient(ptr, mesh->mtl.ambient, material);
      if (face) {
        ptr += copy_ambient(ptr, mesh->mtl.ambient, material);
      } else if (cur->num > 3) {
        ptr += copy_ambient(ptr, mesh->mtl.ambient, material);
        ptr += copy_ambient(ptr, mesh->mtl.ambient, material);
        ptr += copy_ambient(ptr, mesh->mtl.ambient, material);
      }
      cur = cur->next;
    }
    while (cur) {
      ptr += copy_diffuse(ptr, mesh->mtl.diffuse, material);
      ptr += copy_diffuse(ptr, mesh->mtl.diffuse, material);
      ptr += copy_diffuse(ptr, mesh->mtl.diffuse, material);
      if (face) {
        ptr += copy_diffuse(ptr, mesh->mtl.diffuse, material);
      } else if (cur->num > 3) {
        ptr += copy_diffuse(ptr, mesh->mtl.diffuse, material);
        ptr += copy_diffuse(ptr, mesh->mtl.diffuse, material);
        ptr += copy_diffuse(ptr, mesh->mtl.diffuse, material);
      }
      cur = cur->next;
    }
    while (cur) {
      ptr += copy_specular(ptr, mesh->mtl.specular, mesh->mtl.specular_co, material);
      ptr += copy_specular(ptr, mesh->mtl.specular, mesh->mtl.specular_co, material);
      ptr += copy_specular(ptr, mesh->mtl.specular, mesh->mtl.specular_co, material);
      if (face) {
        ptr += copy_specular(ptr, mesh->mtl.specular, mesh->mtl.specular_co, material);
      } else if (cur->num > 3) {
        ptr += copy_specular(ptr, mesh->mtl.specular, mesh->mtl.specular_co, material);
        ptr += copy_specular(ptr, mesh->mtl.specular, mesh->mtl.specular_co, material);
        ptr += copy_specular(ptr, mesh->mtl.specular, mesh->mtl.specular_co, material);
      }
      cur = cur->next;
    }
  } else { // interleaved
    cur = mesh->first_face;
    while (cur) {
      ptr += copy_vertex(ptr, cur->vertices[0], vertex);
      ptr += copy_texcoord(ptr, cur->texcoords[0], texcoord);
      ptr += copy_normal(ptr, cur->normals[0], normal);
      ptr += copy_material(ptr, &mesh->mtl, material);
      ptr += copy_vertex(ptr, cur->vertices[1], vertex);
      ptr += copy_texcoord(ptr, cur->texcoords[1], texcoord);
      ptr += copy_normal(ptr, cur->normals[1], normal);
      ptr += copy_material(ptr, &mesh->mtl, material);
      ptr += copy_vertex(ptr, cur->vertices[2], vertex);
      ptr += copy_texcoord(ptr, cur->texcoords[2], texcoord);
      ptr += copy_normal(ptr, cur->normals[2], normal);
      ptr += copy_material(ptr, &mesh->mtl, material);
      if (face) {
        ptr += copy_vertex(ptr, cur->vertices[3], vertex);
        ptr += copy_texcoord(ptr, cur->texcoords[3], texcoord);
        ptr += copy_normal(ptr, cur->normals[3], normal);
        ptr += copy_material(ptr, &mesh->mtl, material);
      } else if (cur->num > 3) {
        ptr += copy_vertex(ptr, cur->vertices[2], vertex);
        ptr += copy_texcoord(ptr, cur->texcoords[2], texcoord);
        ptr += copy_normal(ptr, cur->normals[2], normal);
        ptr += copy_material(ptr, &mesh->mtl, material);
        ptr += copy_vertex(ptr, cur->vertices[3], vertex);
        ptr += copy_texcoord(ptr, cur->texcoords[3], texcoord);
        ptr += copy_normal(ptr, cur->normals[3], normal);
        ptr += copy_material(ptr, &mesh->mtl, material);
        ptr += copy_vertex(ptr, cur->vertices[0], vertex);
        ptr += copy_texcoord(ptr, cur->texcoords[0], texcoord);
        ptr += copy_normal(ptr, cur->normals[0], normal);
        ptr += copy_material(ptr, &mesh->mtl, material);
     }
      cur = cur->next;
    }
  }
  *size = num_points * interleaved_size;
  return buf;
}

GLuint ilG_obj_to_gl(ilG_obj_mesh *mesh, GLint *count)
{
  GLuint buf;
  GLfloat *data;
  size_t size, stride;
  
  //stride = sizeof(GLfloat) * 20;
  glGenBuffers(1, &buf);
  glBindBuffer(GL_ARRAY_BUFFER, buf);
  data = ilG_obj_to_vbo(mesh, OBJ_XYZW, OBJ_UVW, OBJ_NORMALS, 
    OBJ_AMBIENT|OBJ_DIFFUSE|OBJ_SPECULAR, OBJ_TRIANGLES, OBJ_INTERLEAVED, 
    &size, &stride);
  *count = size/stride;
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  free(data);
  
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, NULL); // position
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, 
    (void*)(sizeof(GLfloat)*4));    // texcoord
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
    (void*)(sizeof(GLfloat)*7)); //normal
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride,
    (void*)(sizeof(GLfloat)*10)); // ambient
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride,
    (void*)(sizeof(GLfloat)*13)); // diffuse
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, stride,
    (void*)(sizeof(GLfloat)*16)); // specular
  
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);
  glEnableVertexAttribArray(4);
  glEnableVertexAttribArray(5);

  return buf;
}

void ilG_obj_bindmtl(ilG_obj_mtl mtl, GLuint program, const char *ambient, 
  const char *diffuse, const char *specular, const char *specular_co, 
  const char *transparency)
{
  GLuint  ambient_loc       = glGetUniformLocation(program, ambient), 
          diffuse_loc       = glGetUniformLocation(program, diffuse), 
          specular_loc      = glGetUniformLocation(program, specular), 
          specular_co_loc   = glGetUniformLocation(program, specular_co), 
          transparency_loc  = glGetUniformLocation(program, transparency);
  
  glUniform3fv(ambient_loc,     1, mtl.ambient);
  glUniform3fv(diffuse_loc,     1, mtl.diffuse);
  glUniform3fv(specular_loc,    1, mtl.specular);
  glUniform1f(specular_co_loc,  mtl.specular_co);
  glUniform1f(transparency_loc, mtl.transparency);
}

void ilG_obj_bindmtl_defaults(ilG_obj_mtl mtl, GLuint program)
{
  ilG_obj_bindmtl(mtl, program, "ambient", "diffuse", "specular", "specular_co", 
    "transparency");
}