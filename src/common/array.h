#ifndef IL_ARRAY_H
#define IL_ARRAY_H

#define IL_ARRAY(T, name)    \
    struct name {   \
        T *data;            \
        size_t length;      \
        size_t capacity;    \
    }

#define IL_RESIZE(list)                         \
    do {                                        \
        size_t newcap = (list).capacity * 2;    \
        if (newcap < 64) newcap = 64;           \
        size_t size = sizeof((list).data[0]);   \
        void *tmp = calloc(size, newcap);       \
        if ((list).data) {                      \
            memcpy(tmp, (list).data,            \
                size * (list).length);          \
            free((list).data);                  \
        }                                       \
        (list).data = tmp;                      \
        (list).capacity = newcap;               \
    } while(0)

#define IL_INDEX(list, id, out)         \
    do {                                \
        if ((id) < (list).length) {     \
            (out) = (list).data + id;   \
        } else (out) = NULL;            \
    } while(0)

#define IL_SET(list, id, in) \
    do {                                        \
        if ((id) < (list).length) {             \
            (list).data[id] = (in);             \
        } else {                                \
            while ((id) >= (list).capacity) {   \
                IL_RESIZE(list);                \
            }                                   \
            (list).length = (id) + 1;           \
            (list).data[id] = (in);             \
        }                                       \
    } while(0)

#define IL_INDEXORSET(list, id, out, in) \
    do {                            \
       IL_INDEX(list, id, out);    \
        if (!(out)) {               \
            IL_SET(list, id, in);   \
            (out) = &(in);          \
        }                           \
    } while(0)
#define IL_INDEXORZERO(list, id, out)       \
    do {                                    \
        IL_INDEX(list, id, out);            \
        if (!(out)) {                       \
            while ((list).capacity <= (id)) \
                IL_RESIZE(list);            \
            memset((list).data + id, 0,     \
                    sizeof((list).data[0]));\
            (out) = (list).data + id;       \
            if ((list).length <= (id))      \
                (list).length = (id)+1;     \
        }                                   \
    } while(0)

#define IL_APPEND(list, in)                     \
    do {                                        \
        if ((list).length >= (list).capacity) { \
            IL_RESIZE(list);                    \
        }                                       \
        (list).data[(list).length] = (in);      \
        (list).length++;                        \
    } while(0)

#define IL_INSERT(list, id, in)                 \
    do {                                        \
        if ((id) > (list).length) {             \
            IL_APPEND(list, in);                \
        }                                       \
        if ((list).length >= (list).capacity) { \
            IL_RESIZE(list);                    \
        }                                       \
        memmove( (list).data + (id) + 1,        \
                 (list).data + (id),            \
                 (list).length - (id) - 1 );    \
        (list).length++;                        \
        (list).data[id] = (in);                 \
    } while(0)

#define IL_REMOVE(list, id) \
    do { \
        memmove( (list).data + (id), \
                 (list).data + (id) + 1, \
                 --(list).length - (id) ); \
    } while(0)


#endif

