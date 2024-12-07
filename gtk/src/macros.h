#ifndef __UTILS__MACROS_H__
#define __UTILS__MACROS_H__

#include <stddef.h>

#define MACRO_BEGIN     ({
#define MACRO_END       })

#define XQUOTE(x)       #x
#define QUOTE(x)        XQUOTE(x)

#define STRLEN(x)       (sizeof(x) - 1)
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))
#endif

#ifndef MIN
#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#endif
#ifndef MIN
#define MAX(a, b)       ((a) > (b) ? (a) : (b))
#endif

#define P2ALIGNED(x, a) (((x) & ((a) - 1)) == 0)
#define ISP2(x)         P2ALIGNED(x, x)
#define P2ALIGN(x, a)   ((x) & -(a))
#define P2ROUND(x, a)   (-(-(x) & -(a)))
#define P2END(x, a)     (-(~(x) & -(a)))

#define structof(ptr, type, member) \
    ((type *)((char *)ptr - offsetof(type, member)))

//#define likely(expr)    __builtin_expect(!!(expr), 1)
//#define unlikely(expr)  __builtin_expect(!!(expr), 0)

#define barrier()       asm volatile("" : : : "memory")

#define __noreturn      __attribute__((noreturn))
#define __aligned(x)    __attribute__((aligned(x)))
#define __packed        __attribute__((packed))

#define __format_printf(fmt, args) \
    __attribute__((format(printf, fmt, args)))

#endif /* _MACROS_H */
