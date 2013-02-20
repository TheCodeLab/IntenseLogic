#ifndef ILG_TEXTUREUNIT_H
#define ILG_TEXTUREUNIT_H

#include "common/log.h"

// 64 units per type
#define ILG_TUNIT_NONE          (0)
#define ILG_TUNIT_COLOR0        (1)
#define ILG_TUNIT_NORMAL0       (32)
#define ILG_TUNIT_BUMP0         (64)
#define ILG_TUNIT_SHADOW0       (96)
#define ILG_TUNIT_HEIGHT0       (128)
// types that only need 1 unit
#define ILG_TUNIT_PAGETABLE     (160)
#define ILG_TUNIT_VIRTUALTEX    (161)
#define ILG_TUNIT_NUMUNITS      (162)

#define ILG_TUNIT_ACTIVE(context, unit, type)   \
    if ((unit) < (context)->num_texunits) {     \
        (context)->texunits[unit] = (type);     \
        if ((unit) > (context)->num_active)     \
            (context)->num_active = (unit);     \
    } else {                                    \
        il_log(1, "Not enough texture units!"); \
    }

#endif

