#include "bindable.h"

void ilG_bindable_bind(const ilG_bindable *self, void *obj)
{
    if (!self || !self->bind) {
        return;
    }
    self->bind(obj);
}

void ilG_bindable_action(const ilG_bindable *self, void *obj)
{
    if (!self || !self->action) {
        return;
    }
    self->action(obj);
}

void ilG_bindable_unbind(const ilG_bindable *self, void *obj)
{
    if (!self || !self->unbind) {
        return;
    }
    self->unbind(obj);
}

void ilG_bindable_swap(const ilG_bindable **self, void **ptr, void *new)
{
    if (*ptr == new || !new) {
        return;
    }
    if (*self && *ptr) {
        ilG_bindable_unbind(*self, *ptr);
    }
    *ptr = new;
    *self = il_cast(il_typeof(new), "il.graphics.bindable");
    if (*self && *ptr) {
        ilG_bindable_bind(*self, *ptr);
    }
}

