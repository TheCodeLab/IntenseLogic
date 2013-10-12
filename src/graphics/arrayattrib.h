#ifndef ILG_ARRAYATTRIB_H
#define ILG_ARRAYATTRIB_H

enum ilG_arrayattrib {
    ILG_ARRATTR_POSITION,
    ILG_ARRATTR_TEXCOORD,
    ILG_ARRATTR_NORMAL,
    ILG_ARRATTR_AMBIENT,
    ILG_ARRATTR_DIFFUSE,
    ILG_ARRATTR_SPECULAR,
    ILG_ARRATTR_ISTRANSPARENT,
    ILG_ARRATTR_NUMATTRS,
};

#define ILG_SETATTR(flag, attr) (flag |= (1<<(attr)))
#define ILG_TESTATTR(flag, attr) ((flag & (1<<(attr))) == (1<<attr))

#endif

