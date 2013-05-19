#ifndef ILU_ASSERT_H
#define ILU_ASSERT_H

#include "util/log.h"

#define il_return_on_fail(e) il_return_val_on_fail(e, )
#define il_return_null_on_fail(e) il_return_val_on_fail(e, NULL)
#define il_return_val_on_fail(e, v) if (!e) {il_error("assertion " #e " failed"); return v;}

#endif

