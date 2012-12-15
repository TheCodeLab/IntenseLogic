#ifndef IL_ARRAY_H
#define IL_ARRAY_H

#define IL_ARRAY(T, name)    \
    typedef struct name {   \
        T *data;            \
        size_t length;      \
        size_t capacity;    \
    } name

#define IL_RESIZE(list)                         \
    {                                           \
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
    }

#define IL_INDEX(list, id, out)         \
    {                                   \
        if ((id) < (list).length) {     \
            (out) = (list).data + id;   \
        } else (out) = NULL;            \
    }

#define IL_SET(list, id, in) \
    {                                           \
        if ((id) < (list).length)               \
            (list).data[id] = (in);             \
        else {                                  \
            while ((id) <= (list).capacity) {   \
                IL_RESIZE(list);                \
            }                                   \
            (list).length = (id) + 1;           \
            (list).data[id] = (in);             \
        }                                       \
    }

#define IL_INDEXORSET(list, id, out, in) \
    {                               \
       IL_ INDEX(list, id, out);    \
        if (!(out)) {               \
            IL_SET(list, id, in);   \
            (out) = &(in);          \
        }                           \
    }
#define IL_INDEXORZERO(list, id, out)       \
    {                                       \
        IL_INDEX(list, id, out);            \
        if (!(out)) {                       \
            while ((list).capacity <= (id)) \
                IL_RESIZE(list);            \
            memset((list).data + id, 0,     \
                    sizeof((list).data[0]));\
            (out) = (list).data + id;       \
        }                                   \
    }

#define IL_APPEND(list, in)                     \
    {                                           \
        if ((list).length >= (list).capacity) { \
            IL_RESIZE(list);                    \
        }                                       \
        (list).data[(list).length] = (in);      \
        (list).length++;                        \
    }


#endif

