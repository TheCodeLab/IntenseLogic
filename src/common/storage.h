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
#define il_vopaque(data, dtor) il_value_opaque(il_opaque(data, dtor))

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

const il_value  *il_table_getv (const il_table *self, il_value key);
bool             il_table_getvb(const il_table *self, il_value key);
int              il_table_getvi(const il_table *self, il_value key);
float            il_table_getvf(const il_table *self, il_value key);
const void      *il_table_getvp(const il_table *self, il_value key);
const char      *il_table_getvs(const il_table *self, il_value key);
const il_table  *il_table_getvt(const il_table *self, il_value key);
const il_vector *il_table_getva(const il_table *self, il_value key);

const il_value  *il_table_gets (const il_table *self, const char *key);
bool             il_table_getsb(const il_table *self, const char *key);
int              il_table_getsi(const il_table *self, const char *key);
float            il_table_getsf(const il_table *self, const char *key);
const void      *il_table_getsp(const il_table *self, const char *key);
const char      *il_table_getss(const il_table *self, const char *key);
const il_table  *il_table_getst(const il_table *self, const char *key);
const il_vector *il_table_getsa(const il_table *self, const char *key);

const il_value  *il_table_geti (const il_table *self, int key);
bool             il_table_getib(const il_table *self, int key);
int              il_table_getii(const il_table *self, int key);
float            il_table_getif(const il_table *self, int key);
const void      *il_table_getip(const il_table *self, int key);
const char      *il_table_getis(const il_table *self, int key);
const il_table  *il_table_getit(const il_table *self, int key);
const il_vector *il_table_getia(const il_table *self, int key);

il_value    *il_table_mgetv (il_table *self, il_value key);
void        *il_table_mgetvp(il_table *self, il_value key);
char        *il_table_mgetvs(il_table *self, il_value key);
il_table    *il_table_mgetvt(il_table *self, il_value key);
il_vector   *il_table_mgetva(il_table *self, il_value key);

il_value    *il_table_mgets (il_table *self, const char *key);
void        *il_table_mgetsp(il_table *self, const char *key);
char        *il_table_mgetss(il_table *self, const char *key);
il_table    *il_table_mgetst(il_table *self, const char *key);
il_vector   *il_table_mgetsa(il_table *self, const char *key);

il_value    *il_table_mgeti (il_table *self, int key);
void        *il_table_mgetip(il_table *self, int key);
char        *il_table_mgetis(il_table *self, int key);
il_table    *il_table_mgetit(il_table *self, int key);
il_vector   *il_table_mgetia(il_table *self, int key);

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

unsigned il_vector_len(const il_vector *self);

const il_value  *il_vector_get (const il_vector *self, unsigned idx);
bool             il_vector_getb(const il_vector *self, unsigned idx);
int              il_vector_geti(const il_vector *self, unsigned idx);
float            il_vector_getf(const il_vector *self, unsigned idx);
const void      *il_vector_getp(const il_vector *self, unsigned idx);
const char      *il_vector_gets(const il_vector *self, unsigned idx);
const il_table  *il_vector_gett(const il_vector *self, unsigned idx);
const il_vector *il_vector_geta(const il_vector *self, unsigned idx);

il_value    *il_vector_mget (il_vector *self, unsigned idx);
void        *il_vector_mgetp(il_vector *self, unsigned idx);
char        *il_vector_mgets(il_vector *self, unsigned idx);
il_table    *il_vector_mgett(il_vector *self, unsigned idx);
il_vector   *il_vector_mgeta(il_vector *self, unsigned idx);

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
const char      *il_value_strwhich  (const il_value *v);
bool             il_value_tobool    (const il_value *v);
int              il_value_toint     (const il_value *v);
float            il_value_tofloat   (const il_value *v);
const void      *il_value_tovoid    (const il_value *v);
const char      *il_value_tostr     (const il_value *v);
const il_table  *il_value_totable   (const il_value *v);
const il_vector *il_value_tovec     (const il_value *v);

void            *il_value_tomvoid   (il_value *v);
char            *il_value_tomstr    (il_value *v);
il_table        *il_value_tomtable  (il_value *v);
il_vector       *il_value_tomvec    (il_value *v);

void il_value_show(il_value *v);
void il_table_show(il_table *t);

#endif
