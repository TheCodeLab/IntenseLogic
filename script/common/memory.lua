local ffi = require "ffi"

ffi.cdef [[

struct il_GC;

typedef struct il_GC* (*il_GC_copy)(struct il_GC*);
typedef void (*il_GC_free)(struct il_GC*);

typedef struct il_GC {
    il_GC_copy copy;
    il_GC_free free;
    unsigned refs;
    struct il_GC *next;
    size_t size;
    ptrdiff_t baseptr;
} il_GC;

]]

