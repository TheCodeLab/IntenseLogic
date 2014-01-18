#ifndef IL_STORAGE_H
#define IL_STORAGE_H

#include <stdlib.h>
#include <stdbool.h>

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

typedef struct il_storage_void {
    void *data;
    void (*dtor)(void*);
} il_storage_void;

#define il_opaque(data, dtor) ((struct il_storage_void){data, dtor})

typedef struct il_table {
    struct s_key *head;
} il_table;

struct il_value;

typedef IL_ARRAY(struct il_value, il_vector) il_vector;

typedef struct il_value {
    enum il_storagetype tag;
    union {
        int dummy;
        il_storage_void svoid;
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

il_value    *il_table_getv (il_table *self, il_value key);
bool         il_table_getvb(il_table *self, il_value key);
void        *il_table_getvp(il_table *self, il_value key);
char        *il_table_getvs(il_table *self, il_value key);
int          il_table_getvi(il_table *self, il_value key);
float        il_table_getvf(il_table *self, il_value key);
il_table    *il_table_getvt(il_table *self, il_value key);
il_vector   *il_table_getva(il_table *self, il_value key);

il_value    *il_table_gets (il_table *self, const char *key);
bool         il_table_getsb(il_table *self, const char *key);
void        *il_table_getsp(il_table *self, const char *key);
char        *il_table_getss(il_table *self, const char *key);
int          il_table_getsi(il_table *self, const char *key);
float        il_table_getsf(il_table *self, const char *key);
il_table    *il_table_getst(il_table *self, const char *key);
il_vector   *il_table_getsa(il_table *self, const char *key);

il_value    *il_table_geti (il_table *self, int key);
bool         il_table_getib(il_table *self, int key);
void        *il_table_getip(il_table *self, int key);
char        *il_table_getis(il_table *self, int key);
int          il_table_getii(il_table *self, int key);
float        il_table_getif(il_table *self, int key);
il_table    *il_table_getit(il_table *self, int key);
il_vector   *il_table_getia(il_table *self, int key);

il_value *il_table_setv (il_table *self, il_value key, il_value val);
il_value *il_table_setvb(il_table *self, il_value key, bool val);
il_value *il_table_setvp(il_table *self, il_value key, il_storage_void val);
il_value *il_table_setvs(il_table *self, il_value key, const char *val);
il_value *il_table_setvi(il_table *self, il_value key, int val);
il_value *il_table_setvf(il_table *self, il_value key, float val);
il_value *il_table_setvt(il_table *self, il_value key, il_table val);
il_value *il_table_setva(il_table *self, il_value key, il_vector val);

il_value *il_table_sets (il_table *self, const char *key, il_value val);
il_value *il_table_setsb(il_table *self, const char *key, bool val);
il_value *il_table_setsp(il_table *self, const char *key, il_storage_void val);
il_value *il_table_setss(il_table *self, const char *key, const char *val);
il_value *il_table_setsi(il_table *self, const char *key, int val);
il_value *il_table_setsf(il_table *self, const char *key, float val);
il_value *il_table_setst(il_table *self, const char *key, il_table val);
il_value *il_table_setsa(il_table *self, const char *key, il_vector val);

il_value *il_table_seti (il_table *self, int key, il_value val);
il_value *il_table_setib(il_table *self, int key, bool val);
il_value *il_table_setip(il_table *self, int key, il_storage_void val);
il_value *il_table_setis(il_table *self, int key, const char *val);
il_value *il_table_setii(il_table *self, int key, int val);
il_value *il_table_setif(il_table *self, int key, float val);
il_value *il_table_setit(il_table *self, int key, il_table val);
il_value *il_table_setia(il_table *self, int key, il_vector val);

il_vector il_vector_new(size_t num, ...);
il_vector il_vector_newv(size_t num, il_value *v);
void il_vector_free(il_vector *self);
il_value    *il_vector_get (il_vector *self, unsigned idx);
bool         il_vector_getb(il_vector *self, unsigned idx);
void        *il_vector_getp(il_vector *self, unsigned idx);
char        *il_vector_gets(il_vector *self, unsigned idx);
int          il_vector_geti(il_vector *self, unsigned idx);
float        il_vector_getf(il_vector *self, unsigned idx);
il_table    *il_vector_gett(il_vector *self, unsigned idx);
il_vector   *il_vector_geta(il_vector *self, unsigned idx);
il_value *il_vector_set (il_vector *self, unsigned idx, il_value v);
il_value *il_vector_setb(il_vector *self, unsigned idx, bool val);
il_value *il_vector_setp(il_vector *self, unsigned idx, il_storage_void val);
il_value *il_vector_sets(il_vector *self, unsigned idx, const char* str);
il_value *il_vector_seti(il_vector *self, unsigned idx, int val);
il_value *il_vector_setf(il_vector *self, unsigned idx, float val);
il_value *il_vector_sett(il_vector *self, unsigned idx, il_table t);
il_value *il_vector_seta(il_vector *self, unsigned idx, il_vector v);

il_value il_value_nil();
il_value il_value_true();
il_value il_value_false();
#define il_value_bool(b) (b? il_value_true() : il_value_false())
il_value il_value_opaque(il_storage_void v);
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

