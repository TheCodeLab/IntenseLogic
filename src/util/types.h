#ifndef IL_ATTRIBS_H
#define IL_ATTRIBS_H

#include <stdint.h>

#if defined(__GNUC__)

# define IL_WARN_UNUSED __attribute__((warn_unused_result))
# define IL_FORMAT_STRING(x,y) __attribute__((format(printf, x, y)))

#elif defined(_MSC_VER)

# define IL_WARN_UNUSED _Check_return_
# define IL_FORMAT_STRING(x,y)
# define __func__ __FUNCTION__

#else

# define IL_WARN_UNUSED
# define IL_FORMAT_STRING(x,y)

#endif

#endif
