#include "storage.h"

#include <string.h>
#include <stdarg.h>

#include "util/uthash.h"
#include "util/array.h"
#include "util/log.h"

static il_value invalid = {
    IL_INVALID,
    {0}
};

struct s_key {
    il_value key;
    il_value val;
    UT_hash_handle hh;
};

static void *s_hash_data(il_value *v, size_t *size)
{
    switch (v->tag) {
        case IL_INVALID:
        case IL_NIL:
        case IL_TRUE:
        case IL_FALSE:  *size = sizeof(enum il_storagetype);    return &v->tag;
        case IL_VOID:   *size = sizeof(void*);                  return &v->val.svoid.data;
        case IL_STRING: *size = strlen(v->val.string);          return v->val.string;
        case IL_INT:    *size = sizeof(int);                    return &v->val.sint;
        case IL_FLOAT:  *size = sizeof(float);                  return &v->val.sfloat;
        case IL_TABLE:  // cannot be keys because I am lazy
        case IL_VECTOR:
        case IL_LUA:    return NULL;
    }
}

il_table il_table_new()
{
    il_table tab;
    tab.head = NULL;
    return tab;
}

void il_table_free(il_table self)
{
    struct s_key *key, *tmp;
    HASH_ITER(hh, self.head, key, tmp) {
        HASH_DEL(self.head, key);
        il_value_free(key->key);
        il_value_free(key->val);
        free(key);
    }
}

il_value *il_table_setv(il_table *self, il_value key, il_value val)
{
    size_t size;
    void *data = s_hash_data(&key, &size);
    struct s_key *item = NULL;

    if (!data) {
        il_error("Invalid key type: %s not supported", il_value_strwhich(&key));
        return &invalid;
    }
    HASH_FIND(hh, self->head, data, size, item);
    if (item) {
        HASH_DEL(self->head, item);
        il_value_free(item->key);
        il_value_free(item->val);
        free(item);
    }
    item = calloc(1, sizeof(struct s_key));
    item->key = key;
    item->val = val;
    HASH_ADD_KEYPTR(hh, self->head, data, size, item);
    return &item->val;
}

il_value *il_table_sets(il_table *self, const char *key, il_value val)
{
    char *end = strchr(key, '.');
    il_value keyv;
    if (end) { // we have not descended far enough yet
        keyv = il_value_slice(key, end-key);
        il_value *t = il_table_getv(self, keyv);
        if (il_value_which(t) == IL_INVALID) { // create non-existent nodes, like mkdir -p
            il_table tab = il_table_new();
            il_value v = il_value_table(tab);
            t = il_table_setv(self, keyv, v);
        }
        if (il_value_which(t) != IL_TABLE) {
            il_error("Expected table, got %s", il_value_strwhich(t));
            return &invalid;
        }
        il_table *tab = il_value_totable(t);
        return il_table_sets(tab, end+1, val);
    } else { // finally set the value
        keyv = il_value_string(key);
        return il_table_setv(self, keyv, val);
    }
}

il_value *il_table_seti(il_table *self, int key, il_value val)
{
    return il_table_setv(self, il_value_int(key), val);
}

il_value *il_table_getv(il_table *self, il_value key)
{
    size_t size;
    void *data = s_hash_data(&key, &size);
    if (!data) {
        il_error("Invalid key type: %s not supported", il_value_strwhich(&key));
        return &invalid;
    }
    struct s_key *item;
    HASH_FIND(hh, self->head, data, size, item);
    if (item) {
        return &item->val;
    }
    return &invalid;
}

il_value *il_table_gets(il_table *self, const char *key)
{
    char *end = strchr(key, '.');
    il_value keyv;
    if (end) { // we have not descended far enough yet
        keyv = il_value_slice(key, end-key);
        il_value *t = il_table_getv(self, keyv);
        if (il_value_which(t) == IL_INVALID) {
            return &invalid; // suppress error
        }
        if (il_value_which(t) != IL_TABLE) {
            il_error("Expected table, got %s", il_value_strwhich(t));
            return &invalid;
        }
        il_table *tab = il_value_totable(t);
        return il_table_gets(tab, end+1);
    } else { // finally set the value
        keyv = il_value_string(key);
        return il_table_getv(self, keyv);
    }
}

il_value *il_table_geti(il_table *self, int key)
{
    return il_table_getv(self, il_value_int(key));
}

il_vector il_vector_new(size_t num, ...)
{
    unsigned i;
    va_list ap;
    il_vector a = {0, 0, 0};

    va_start(ap, num);
    for (i = 0; i < num; i++) {
        IL_APPEND(a, va_arg(ap, il_value));
    }
    va_end(ap);

    return a;
}

il_vector il_vector_newv(size_t num, il_value *v)
{
    il_vector a = {0, 0, 0};
    unsigned i;

    for (i = 0; i < num; i++) {
        IL_APPEND(a, v[i]);
    }
    return a;
}

il_value *il_vector_get(il_vector *self, unsigned idx)
{
    if (idx >= self->length) {
        il_error("Index %u out of bounds (size %zu)", idx, self->length);
        return &invalid;
    }
    return &self->data[idx];
}

#define getfunc(T, l, op) \
    T il_vector_get##l(il_vector *self, unsigned idx) \
    { \
        return op(il_vector_get(self, idx));\
    }

getfunc(void*,      p, il_value_tovoid)
getfunc(char*,      s, il_value_tostr)
getfunc(int,        i, il_value_toint)
getfunc(float,      f, il_value_tofloat)
getfunc(il_table*,  t, il_value_totable)
getfunc(il_vector*, v, il_value_tovec)

il_value *il_vector_set(il_vector *self, unsigned idx, il_value v)
{
    IL_SET((*self), idx, v);
    return &self->data[idx];
}

#define setfunc(T, l, op) \
    il_value *il_vector_set##l(il_vector *self, unsigned idx, T val) \
    { \
        return il_vector_set(self, idx, op(val)); \
    }

//setfunc(void*, p, il_value_opaque)
il_value *il_vector_setp(il_vector *self, unsigned idx, void *data, void (*dtor)(void*))
{
    return il_vector_set(self, idx, il_value_opaque(data, dtor));
}
setfunc(const char*, s, il_value_string)
setfunc(int, i, il_value_int)
setfunc(float, f, il_value_float)
setfunc(il_table, t, il_value_table)
setfunc(il_vector, v, il_value_vector)

void il_vector_free(il_vector *v)
{
    unsigned i;
    for (i = 0; i < v->length; i++) {
        il_value_free(v->data[i]);
    }
    IL_FREE((*v));
}

il_value il_value_nil()
{
    il_value v;
    v.tag = IL_NIL;
    return v;
}

il_value il_value_true()
{
    il_value v;
    v.tag = IL_TRUE;
    return v;
}

il_value il_value_false()
{
    il_value v;
    v.tag = IL_FALSE;
    return v;
}

il_value il_value_opaque(void *data, void(*dtor)(void*))
{
    il_value val;
    val.tag = IL_VOID;
    val.val.svoid.data = data;
    val.val.svoid.dtor = dtor;
    return val;
}

char *strdup(const char*);
il_value il_value_string(const char* str)
{
    il_value val;
    val.tag = IL_STRING;
    val.val.string = strdup(str);
    return val;
}

il_value il_value_slice(const char *str, size_t len)
{
    il_value val;
    val.tag = IL_STRING;
    char *buf = malloc(len+1);
    memcpy(buf, str, len);
    buf[len] = 0;
    val.val.string = buf;
    return val;
}

il_value il_value_int(int v)
{
    il_value val;
    val.tag = IL_INT;
    val.val.sint = v;
    return val;
}

il_value il_value_float(float v)
{
    il_value val;
    val.tag = IL_FLOAT;
    val.val.sfloat = v;
    return val;
}

il_value il_value_table(il_table t)
{
    il_value val;
    val.tag = IL_TABLE;
    val.val.table = t;
    return val;
}

il_value il_value_vector(il_vector v)
{
    il_value  val;
    val.tag = IL_VECTOR;
    val.val.vector = v;
    return val;
}

il_value il_value_vectorl(size_t num, ...)
{
    va_list ap;
    il_vector a = {0, 0, 0};
    unsigned i;
    il_value v;

    va_start(ap, num);
    for (i = 0; i < num; i++) {
        IL_APPEND(a, va_arg(ap, il_value));
    }
    va_end(ap);
    v.tag = IL_VECTOR;
    v.val.vector = a;
    return v;
}

il_value il_value_copy(il_value *v)
{
    switch (v->tag) {
        case IL_INVALID:
        case IL_NIL:
        case IL_TRUE:
        case IL_FALSE:
        return *v;
        case IL_VOID:
        il_error("Cannot copy void");
        return il_value_nil();
        case IL_STRING:
        return il_value_string(v->val.string);
        case IL_INT:
        case IL_FLOAT:
        return *v;
        case IL_TABLE:
        case IL_VECTOR:
        case IL_LUA:
        il_error("Copying of %s is not yet implemented", il_value_strwhich(v));
        return il_value_nil();
    }
}

void il_value_free(il_value v)
{
    switch (v.tag) {
        case IL_VOID:   if (v.val.svoid.dtor) v.val.svoid.dtor(v.val.svoid.data); return;
        case IL_STRING: free(v.val.string); return;
        case IL_TABLE:  il_table_free(v.val.table); return;
        case IL_VECTOR: il_vector_free(&v.val.vector); return;
        default: return;
    }
}

const char *il_value_strwhich(const il_value *v)
{
    switch (v->tag) {
        case IL_INVALID:    return "invalid";
        case IL_NIL:        return "nil";
        case IL_TRUE:
        case IL_FALSE:      return "bool";
        case IL_VOID:       return "void";
        case IL_STRING:     return "string";
        case IL_INT:        return "int";
        case IL_FLOAT:      return "float";
        case IL_TABLE:      return "table";
        case IL_VECTOR:     return "vector";
        case IL_LUA:        return "lua";
    }
}

int il_value_tobool(const il_value *v)
{
    if (v->tag == IL_TRUE) return 1;
    if (v->tag == IL_FALSE) return 0;
    return -1;
}

void *il_value_tovoid(il_value *v)
{
    if (v->tag != IL_VOID) return NULL;
    return v->val.svoid.data;
}

char *il_value_tostr(il_value *v)
{
    if (v->tag != IL_STRING) return NULL;
    return v->val.string;
}

int il_value_toint(const il_value *v)
{
    if (v->tag != IL_INT) return 0;
    return v->val.sint;
}

float il_value_tofloat(const il_value *v)
{
    if (v->tag != IL_FLOAT) return 0.0;
    return v->val.sfloat;
}

il_table *il_value_totable(il_value *v)
{
    if (v->tag != IL_TABLE) return NULL;
    return &v->val.table;
}

il_vector *il_value_tovec(il_value *v)
{
    if (v->tag != IL_VECTOR) return NULL;
    return &v->val.vector;
}

static void print_indent(unsigned indent)
{
    unsigned i;
    fputc('\n', stderr);
    for (i = 0; i < indent; i++) {
        fputc('\t', stderr);
    }
}

static void show(il_value *v, unsigned indent);

static void show_table(il_table *t, unsigned indent)
{
    struct s_key *item, *tmp;
    fputc('{', stderr);
    HASH_ITER(hh, t->head, item, tmp) {
        print_indent(indent+1);
        show(&item->key, indent+1);
        fputs(": ", stderr);
        show(&item->val, indent+1);
    }
    print_indent(indent);
    fputc('}', stderr);
}

static void show(il_value *v, unsigned indent)
{
    unsigned i;
    switch (v->tag) {
        case IL_INVALID:
        case IL_NIL:
        case IL_TRUE:
        case IL_FALSE:
        case IL_LUA:
        fprintf(stderr, "%s", il_value_strwhich(v));
        break;
        case IL_VOID:
        fprintf(stderr, "void(%p)", v->val.svoid.data);
        break;
        case IL_STRING:
        fprintf(stderr, "\"%s\"", v->val.string);
        break;
        case IL_INT:
        fprintf(stderr, "%i", v->val.sint);
        break;
        case IL_FLOAT:
        fprintf(stderr, "%.6f", v->val.sfloat);
        break;
        case IL_TABLE:
        show_table(&v->val.table, indent);
        break;
        case IL_VECTOR: {
            fputc('[', stderr);
            for (i = 0; i < v->val.vector.length; i++) {
                if (i > 0) {
                    fputc(',', stderr);
                }
                print_indent(indent+1);
                show(&v->val.vector.data[i], indent+1);
            }
            print_indent(indent);
            fputc(']', stderr);
            break;
        }
        default:
        fputs("<garbage>", stderr);
    }
}

void il_value_show(il_value *v)
{
    show(v, 0);
    fputc('\n', stderr);
}

void il_table_show(il_table *t)
{
    show_table(t, 0);
    fputc('\n', stderr);
}

