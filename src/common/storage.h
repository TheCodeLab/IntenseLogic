#ifndef IL_STORAGE_H
#define IL_STORAGE_H

#include <stdlib.h>

#include "util/array.h"

enum il_storagetype {
    IL_INVALID,
    IL_NIL,
    IL_TRUE,
    IL_FALSE,
    IL_VOID,                /**< s_void */
    IL_STRING,              /**< Null terminated C string */
    IL_INT,                 /**< C int type */
    IL_FLOAT,               /**< C float type */
    IL_TABLE,               /**< Nested key-value storage */
    IL_VECTOR,
    IL_LUA,                 /**< The value is the result of luaL_ref() */
};

struct il_storage_void {
    void *data;
    void (*dtor)(void*);
};

typedef struct il_table {
    struct s_key *head;
} il_table;

struct il_value;

typedef IL_ARRAY(struct il_value, il_vector) il_vector;

typedef struct il_value {
    enum il_storagetype tag;
    union {
        int dummy;
        struct il_storage_void svoid;
        char *string;
        int sint;
        float sfloat;
        il_table table;
        il_vector vector;
        int lua;
    } val;
} il_value;

il_table il_table_new();
void il_table_free(il_table self);
il_value *il_table_setv(il_table *self, il_value key, il_value val);
il_value *il_table_sets(il_table *self, const char *key, il_value val);
il_value *il_table_seti(il_table *self, int key, il_value val);
il_value *il_table_getv(il_table *self, il_value key);
il_value *il_table_gets(il_table *self, const char *key);
il_value *il_table_geti(il_table *self, int key);

il_vector il_vector_new(size_t num, ...);
il_vector il_vector_newv(size_t num, il_value *v);
void il_vector_free(il_vector *self);
il_value    *il_vector_get(il_vector *self, unsigned idx);
void        *il_vector_getp(il_vector *self, unsigned idx);
char        *il_vector_gets(il_vector *self, unsigned idx);
int          il_vector_geti(il_vector *self, unsigned idx);
float        il_vector_getf(il_vector *self, unsigned idx);
il_table    *il_vector_gett(il_vector *self, unsigned idx);
il_vector   *il_vector_getv(il_vector *self, unsigned idx);
il_value *il_vector_set(il_vector *self, unsigned idx, il_value v);
//il_value *il_vector_setp
il_value *il_vector_sets(il_vector *self, unsigned idx, const char* str);
il_value *il_vector_seti(il_vector *self, unsigned idx, int val);
il_value *il_vector_setf(il_vector *self, unsigned idx, float val);
il_value *il_vector_sett(il_vector *self, unsigned idx, il_table t);
il_value *il_vector_setv(il_vector *self, unsigned idx, il_vector v);

il_value il_value_nil();
il_value il_value_true();
il_value il_value_false();
#define il_value_bool(b) (b? il_value_true() : il_value_false())
il_value il_value_opaque(void *data, void(*dtor)(void*));
il_value il_value_string(const char*);
il_value il_value_slice(const char*, size_t);
il_value il_value_int(int);
il_value il_value_float(float);
il_value il_value_table(il_table);
il_value il_value_vector(il_vector);
il_value il_value_vectorl(size_t num, ...);
il_value il_value_copy(il_value *v);
void il_value_free(il_value v);
#define il_value_which(v) (v->tag)
const char  *il_value_strwhich(const il_value *v);
int          il_value_tobool(const il_value *v);
void        *il_value_tovoid(il_value *v);
char        *il_value_tostr(il_value *v);
int          il_value_toint(const il_value *v);
float        il_value_tofloat(const il_value *v);
il_table    *il_value_totable(il_value *v);
il_vector   *il_value_tovec(il_value *v);
void il_value_show(il_value *v);
void il_table_show(il_table *t);

#endif

