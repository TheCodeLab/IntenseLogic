/** @file array.h
 * @brief Dynamically resizable arrays implemented entirely in C preprocessor
 */

#ifndef IL_ARRAY_H
#define IL_ARRAY_H

#include <stdlib.h>
#include <string.h>

/** Use this macro to define an array, the name parameter is only used if you wish to give a name to the created type */
#define IL_ARRAY(T, name)   \
    struct name {           \
        T *data;            \
        size_t length;      \
        size_t capacity;    \
    }

#ifdef __cplusplus
#define IL_ARRAY_INTERNAL_CAST(x, y) decltype(x)(y)
#else
#define IL_ARRAY_INTERNAL_CAST(x, y) y
#endif

#define IL_RESIZE(list, nmemb)                  \
    do {                                        \
        size_t size = sizeof((list).data[0]);   \
        void *tmp = calloc(size, nmemb);        \
        if ((list).data) {                      \
            size_t min = (list).length > (nmemb)? \
                (nmemb) : (list).length;        \
            memcpy(tmp, (list).data,            \
                    size * min);                \
            free((list).data);                  \
        }                                       \
        (list).data = IL_ARRAY_INTERNAL_CAST(list.data, tmp);    \
        (list).capacity = nmemb;                \
    } while(0)

/** Doubles the size of an array */
#define IL_AUTORESIZE(list)                     \
    do {                                        \
        size_t newcap = (list).capacity * 2;    \
        if (newcap < 64) {                      \
            newcap = 64;                        \
        }                                       \
        IL_RESIZE(list, newcap);                \
    } while(0)

/** Clears all data used by the array */
#define IL_FREE(list)           \
    do {                        \
        if (list.data) {        \
            free(list.data);    \
        }                       \
    } while(0)

/** Does a bounds-checked access to the array */
#define IL_INDEX(list, id, out)         \
    do {                                \
        if ((id) < (list).length) {     \
            (out) = (list).data + id;   \
        } else (out) = NULL;            \
    } while(0)

/** Sets the specified array index to the value, resizing as necessary */
#define IL_SET(list, id, in) \
    do {                                        \
        if ((id) < (list).length) {             \
            (list).data[id] = (in);             \
        } else {                                \
            while ((id) >= (list).capacity) {   \
                IL_AUTORESIZE(list);            \
            }                                   \
            (list).length = (id) + 1;           \
            (list).data[id] = (in);             \
        }                                       \
    } while(0)

/** Retrieves the specified index, and if it doesn't exist, then set it */
#define IL_INDEXORSET(list, id, out, in) \
    do {                            \
       IL_INDEX(list, id, out);     \
        if (!(out)) {               \
            IL_SET(list, id, in);   \
            (out) = &(in);          \
        }                           \
    } while(0)

/** Retrieves the specified index, and if it doesn't exist, zero it */
#define IL_INDEXORZERO(list, id, out)           \
    do {                                        \
        IL_INDEX(list, id, out);                \
        if (!(out)) {                           \
            while ((list).capacity <= (id)) {   \
                size_t max = (id) > (list).capacity * 2? \
                    (id) : (list).capacity;     \
                IL_RESIZE(list, max);           \
            }                                   \
            memset((list).data + id, 0,         \
                    sizeof((list).data[0]));    \
            (out) = (list).data + id;           \
            if ((list).length <= (id))          \
                (list).length = (id)+1;         \
        }                                       \
    } while(0)

/** Appends an element to the end of the array, resizing if necessary */
#define IL_APPEND(list, in)                     \
    do {                                        \
        if ((list).length >= (list).capacity) { \
            IL_AUTORESIZE(list);                \
        }                                       \
        (list).data[(list).length++] = (in);    \
    } while(0)

/** Inserts an element at the specified index, resizing if necessary */
#define IL_INSERT(list, id, in)                 \
    do {                                        \
        if ((id) > (list).length) {             \
            IL_APPEND(list, in);                \
        }                                       \
        if ((list).length >= (list).capacity) { \
            IL_AUTORESIZE(list);                \
        }                                       \
        memmove( (list).data + (id) + 1,        \
                 (list).data + (id),            \
                 sizeof((list).data[0]) *       \
                 ((list).length - (id)) );      \
        (list).length++;                        \
        (list).data[id] = (in);                 \
    } while(0)

/** Removes the element at the specified index, shuffling all the items after it back one. Do not use this if your array doesn't have to remain sorted. */
#define IL_REMOVE(list, id) \
    do { \
        memmove( (list).data + (id), \
                 (list).data + (id) + 1, \
                 --(list).length - (id) ); \
    } while(0)

/** Shuffles the last element into the position of the to-be-removed element */
#define IL_FASTREMOVE(list, id)                                 \
    do {                                                        \
        if ((id) < (list).length-1) { /* not the last element */\
            (list).data[id] = (list).data[--(list).length];     \
        } else {                                                \
            --(list).length;                                    \
        }                                                       \
    } while(0)

#define IL_FIND(list, var, eq, out_id)  \
    do {                                \
        for (out_id = 0; out_id < (list).length; out_id++) { \
            var = (list).data[out_id];  \
            if (eq) {                   \
                break;                  \
            }                           \
        }                               \
    } while(0)

#endif
