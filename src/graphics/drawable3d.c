#include "drawable3d.h"

#include <stdlib.h>
#include <string.h>

#include "uthash.h"

struct hash_entry {
    il_positionable* ptr;
    size_t len;
    ilG_drawable3d** data;
    UT_hash_handle hh;
};

static struct hash_entry *head = NULL;

struct ilG_drawable3dIterator {
    size_t n;
};

void ilG_drawable3d_setPositionable(ilG_drawable3d* self,
        il_positionable* pos)
{
    struct hash_entry * entry = NULL;
    size_t i;
    ilG_drawable3d** temp;

    // Step 1: Remove the old reference
    HASH_FIND_PTR(head, &self->positionable, entry);
    if (entry != NULL) {
        for (i = 0; i < entry->len; i++) {
            if (entry->data[i] == self) {
                entry->data[i] = entry->data[--entry->len];
                entry->data = realloc(entry->data,
                                      sizeof(ilG_drawable3d*) * entry->len);
            }
        }
    }
    // Step 2: Add a new reference
    HASH_FIND_PTR(head, &pos, entry);
    if (entry == NULL) {
        // allocate a new entry
        entry = calloc(1, sizeof(struct hash_entry));
        entry->ptr = pos;
        entry->data = malloc(0);
        HASH_ADD_PTR(head, ptr, entry);
    }
    temp = calloc(++entry->len, sizeof(ilG_drawable3d));
    memcpy(temp, entry->data, entry->len);
    free(entry->data);
    entry->data = temp;
    entry->data[entry->len-1] = self;
    // Step 3: Set the positionable of the drawable
    self->positionable = pos;
}

ilG_drawable3d* ilG_drawable3d_iterate(il_positionable*
        pos, ilG_drawable3dIterator** iter)
{
    struct hash_entry * entry;

    HASH_FIND_PTR(head, &pos, entry);
    if (entry == NULL)
        return NULL;

    if (*iter == NULL)
        *iter = calloc(1, sizeof(ilG_drawable3dIterator));

    if ((*iter)->n > entry->len) return NULL;

    return entry->data[(*iter)->n++];
}
