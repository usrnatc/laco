#if !defined(__COMMON_H__)
#define __COMMON_H__

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uintptr_t uptr;
typedef float f32;
typedef double f64;
typedef u8 b8;

#define ARRAY_COUNT(X) (sizeof((X)) / sizeof(*(X)))

#define internal static
#define global static
#define local static

#ifndef TRUE
    #define TRUE 1
    #define FALSE 0
#endif // TRUE

#endif // __COMMON_H__