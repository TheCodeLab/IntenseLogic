#include "serializable.h"

unsigned char *il_serializable_serialize(void *obj, size_t *size)
{
    const il_serializable *iface = il_cast(il_typeof(obj), "il.serializable");
    return iface->serialize(obj, size);
}

void il_serializable_deserialize(void *obj, unsigned char *data, size_t size)
{
    const il_serializable *iface = il_cast(obj, "il.serializable");
    iface->deserialize(obj, data, size);
}

unsigned char *il_serializable_diff(void *obj1, void *obj2, size_t *size)
{
    const il_serializable *iface = il_cast(il_typeof(obj1), "il.serializable");
    return iface->diff(obj1, obj2, size);
}

void il_serializable_merge(void *obj, unsigned char *data, size_t size, struct timeval arrival)
{
    const il_serializable *iface = il_cast(il_typeof(obj), "il.serializable");
    iface->merge(obj, data, size, arrival);
}

