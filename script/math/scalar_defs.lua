local ffi = require "ffi"

ffi.cdef [[

typedef struct il_vec2 {
  float x, y;
}  __attribute__((aligned(16), packed)) il_vec2;
typedef struct il_vec4 {
  float x, y, z, w;
}  __attribute__((aligned(16), packed)) il_vec4;
typedef il_vec4 il_vec3;

typedef struct il_quat {
  float x, y, z, w;
}  __attribute__((aligned(16), packed)) il_quat;
typedef struct il_mat {
  float data[16];
}  __attribute__((aligned(16), packed)) il_mat;


]]

