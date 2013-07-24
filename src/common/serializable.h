/** @file serializable.h
 * @brief Typeclass for serialization
 */

#ifndef IL_SERIALIZABLE_H
#define IL_SERIALIZABLE_H

#include <stdlib.h>
#include <sys/time.h>

#include "common/base.h"

typedef struct il_serializable {
    il_typeclass_header;
    unsigned char *(*serialize)(void *obj, size_t *size);
    void (*deserialize)(void *obj, unsigned char *data, size_t size);
    unsigned char *(*diff)(void *obj1, void *obj2, size_t *size);
    void (*merge)(void *obj1, unsigned char *data, size_t size, struct timeval arrival);
} il_serializable;

/** Serializes an object and returns a buffer that must be freed by the user */
unsigned char *il_serializable_serialize(void *obj, size_t *size);
/** Loads data into an object from some data */
void il_serializable_deserialize(void *obj, unsigned char *data, size_t size);
/** Creates a diff and returns it as a buffer which must be freed by the user */
unsigned char *il_serializable_diff(void *obj1, void *obj2, size_t *size);
/** Uses the diff from il_serializable_diff() to update the specified object */
void il_serializable_merge(void *obj, unsigned char *data, size_t size, struct timeval arrival);

#endif

