#ifndef ILA_FS_H
#define ILA_FS_H

#include "asset/node.h"

ilA_dir *ilA_fs_getRoot();
void ilA_fs_register(const char *prefix, ilA_dir *dir, int priority);

#endif

