/** @file assert.h
 * @brief Provides some assertion-checking macros
 */

#ifndef ILU_ASSERT_H
#define ILU_ASSERT_H

#include "util/log.h"

/** If e evaluates to false, then print an error containing the expression, and return from the function */
#define il_return_on_fail(e) il_return_val_on_fail(e, )
/** If e evaluates to false, then print an error containing the expression, and return NULL */
#define il_return_null_on_fail(e) il_return_val_on_fail(e, NULL)
/** If e evaluates to false, then pritn an error containing the expression, and return v */
#define il_return_val_on_fail(e, v) if (!(e)) {il_error("assertion " #e " failed"); return v;}

#endif

