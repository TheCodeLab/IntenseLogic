local ffi = require "ffi"

ffi.cdef [[

typedef struct {float x,y;} il_Vector2;

typedef struct {float x,y,z;} il_Vector3;

typedef struct {float x,y,z,w;} il_Vector4;

typedef struct {float x,y,z,w;} il_Quaternion;

typedef struct {float data[16];} il_Matrix;

]]

