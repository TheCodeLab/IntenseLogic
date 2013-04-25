#ifndef ILG_BINDABLE_H
#define ILG_BINDABLE_H

#include "common/base.h"

typedef struct ilG_bindable ilG_bindable;

struct ilG_bindable {
    il_typeclass_header;
    void (*bind)(void* obj);
    void (*action)(void* obj);
    void (*unbind)(void* obj);
};

void ilG_bindable_bind(const ilG_bindable *self, void *obj);
void ilG_bindable_action(const ilG_bindable *self, void *obj);
void ilG_bindable_unbind(const ilG_bindable *self, void *obj);

void ilG_bindable_swap(const ilG_bindable **self, void **ptr, void *new);

#endif

