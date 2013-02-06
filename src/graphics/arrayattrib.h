#ifndef ILG_ARRAYATTRIB_H
#define ILG_ARRAYATTRIB_H

#define ILG_ARRATTR_POSITION    (0)
#define ILG_ARRATTR_TEXCOORD    (1)
#define ILG_ARRATTR_NORMAL      (2)
#define ILG_ARRATTR_AMBIENT     (3)
#define ILG_ARRATTR_DIFFUSE     (4)
#define ILG_ARRATTR_SPECULAR    (5)

#define ILG_SETATTR(flag, attr) (flag |= (1<<(attr)))
#define ILG_TESTATTR(flag, attr) ((flag & (1<<(attr))) == (1<<attr))

#endif

