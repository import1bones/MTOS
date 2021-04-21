#ifndef MOS_INCLUDE_TYPES_H
#define MOS_INCLUDE_TYPES_H

#ifndef NULL
#define NULL ((void*) 0)
#endif

typedef _Bool bool;
enum{ false, true };

typedef __signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
typedef uint32_t physaddr_t;

typedef uint32_t page_number_t;
typedef uint32_t size_t;

typedef int32_t ssize_t;

typedef int32_t offset_t;

#define MIN(_a, _b) \
({  \
    typeof(_a) __a = (_a);  \
    typeof(_b) __b = (_b);  \
    __a <= __b ? __a : __b; \
})

#define MAX(_a, _b) \
({  \
    typeof(_a) __a = (_a);  \
    typeof(_b) __b = (_b);  \
    __a <= __b ? __a : __b; \
})

#define ROUNDDOWN(a, n) \
({  \
    uint32_t __a = (uint32_t) (a);  \
    (typeof(a)) (__a - __a % (n));  \
})

#define ROUNDUP(a, n)   \
({  \
    uint32_t __a = (uint32_t) (n);  \
    (typeof(a)) (ROUNDDOWN((uint32_t) (a) + __n - 1, __n)); \
})

#define ARRAY_SIZE(a)   (sizeof(a) / sizeof(a[0]))

#define offsetof(type, member) ((size_t) (&((type*)0)->member))

#endif
