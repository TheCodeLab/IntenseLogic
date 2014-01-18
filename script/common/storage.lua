local ffi = require 'ffi'

ffi.cdef [[

enum il_storagetype {
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

typedef struct il_vector {
    struct il_value *data;
    size_t length;
    size_t capacity;
} il_vector;

typedef struct il_value {
    enum il_storagetype tag;
    union {
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
il_value *il_table_getv(il_table *self, il_value key);

il_vector il_vector_new(size_t num, ...);
il_vector il_vector_newv(size_t num, il_value *v);
void il_vector_free(il_vector *self);
il_value    *il_vector_get(il_vector *self, unsigned idx);
il_value *il_vector_set(il_vector *self, unsigned idx, il_value v);

il_value il_value_nil();
il_value il_value_true();
il_value il_value_false();
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

]]

local value = {}

function value.unpack(v)
    if v == nil then return nil end
    local w = ffi.string(modules.common.il_value_strwhich(v))
    if w == 'nil' then
        return nil
    elseif w == 'bool' then
        return modules.common.il_value_tobool(v)
    elseif w == 'void' then
        return modules.common.il_value_tovoid(v)
    elseif w == 'string' then
        return ffi.string(modules.common.il_value_tostr(v))
    elseif w == 'int' then
        return tonumber(modules.common.il_value_toint(v))
    elseif w == 'float' then
        return tonumber(modules.common.il_value_tofloat(v))
    elseif w == 'table' then
        return modules.common.il_value_totable(v)
    elseif w == 'vector' then
        return modules.common.il_value_tovec(v)
    end
end

function value.pack(v)
    local packed
    if type(v) == 'nil' then
        packed = modules.common.il_value_nil()
    elseif type(v) == 'boolean' then
        packed = v and modules.common.il_value_true() or modules.common.il_value_false()
    elseif type(v) == 'string' then
        packed = modules.common.il_value_string(v)
    elseif type(v) == 'number' then
        packed = modules.common.il_value_float(v)
    elseif type(v) == 'table' then
        local t = modules.common.il_table_new()
        for i, v in pairs(v) do
            t[i] = v
        end
        packed = t
    elseif type(v) == 'cdata' then
        packed = modules.common.il_value_opaque(v, nil)
    else
        error("Don't know how to handle "..type(v))
    end
    return ffi.gc(packed, modules.common.il_value_free)
end

ffi.metatype('il_table', {
    __index = function(t, k)
        return value.unpack(modules.common.il_table_getv(t, value.pack(k)))
    end,
    __newindex = function(t, k, v)
        if not ffi.istype('il_value', v) then
            v = value.pack(v)
        end
        modules.common.il_table_setv(t, ffi.gc(value.pack(k), nil), ffi.gc(v, nil))
    end
})

ffi.metatype('il_value', {
    __index = function(t, k)
        if k == 'type' then
            return ffi.string(modules.common.il_value_strwhich(t))
        end
        if t.type == 'table' or t.type == 'vector' then
            return value.unpack(t)[k]
        end
    end,
    __newindex = function(t, k, v)
        if t.type == 'table' or t.type == 'vector' then
            value.unpack(t)[k] = v
        end
    end
})

return value

