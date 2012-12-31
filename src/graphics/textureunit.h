#ifndef ILG_TEXTUREUNIT_H
#define ILG_TEXTUREUNIT_H

// 64 units per type
#define ILG_TUNIT_NONE      (0)
#define ILG_TUNIT_COLOR0    (1<<0)
#define ILG_TUNIT_NORMAL0   (1<<6)
#define ILG_TUNIT_BUMP0     (1<<12)
#define ILG_TUNIT_SHADOW0   (1<<18)
#define ILG_TUNIT_HEIGHT0   (1<<24)

#define ILG_TUNIT_ACTIVE(context, unit, type)   \
    if ((unit) < (context)->num_texunits) {     \
        (context)->texunits[unit] = (type);     \
        if ((unit) > (context)->num_active)     \
            (context)->num_active = (unit);     \
    } else {                                    \
        il_log(1, "Not enough texture units!"); \
    }

#endif

