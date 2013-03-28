#include "log.h"

#include <mowgli.h>

const char *il_log_prefixes[] = {
    "fatal: ",
    "error: ",
    "warning: ",
    "",
    "debug: "
};


mowgli_patricia_t *dict = NULL;

int il_can_log(const char *file, int level)
{
    if (!dict) {
        return level <= 3;
    }
    char *s = strdup(file);
    if (strchr(s, '/')) {
        *strchr(s, '/') = 0;
    }
    int *mlevel = mowgli_patricia_retrieve(dict, s);
    free(s);
    if (!mlevel) {
        return level <= 3;
    } else {
        return level <= *mlevel;
    }
}

void il_log_toggle(const char *module, int level)
{
    if (!dict) {
        dict = mowgli_patricia_create(NULL);
    }
    int *data;
    if ((data = mowgli_patricia_delete(dict, module)) == NULL) {
        data = calloc(1, sizeof(int));
    }
    *data = level;
    mowgli_patricia_add(dict, module, data);
}

