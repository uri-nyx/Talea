#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

/* TYPE SHORTHANDS */
typedef uint8_t  u8;
typedef int8_t   i8;
typedef uint16_t u16;
typedef int16_t  i16;
typedef uint32_t u32;
typedef int32_t  i32;
typedef uint64_t u64;
typedef int64_t  i64;

#ifdef _WIN32
typedef unsigned long long talea_net_t;
#else
typedef int talea_net_t;
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a)    ((a) > 0 ? (a) : -(a))

// endianness helpers
static inline u32 toBe32(u32 u)
{
    return (u << 24) | ((u & 0x0000FF00) << 8) | ((u & 0x00FF0000) >> 8) | (u >> 24);
}

static inline u32 fromBe32(u32 u)
{
    return (u << 24) | ((u & 0x0000FF00) << 8) | ((u & 0x00FF0000) >> 8) | (u >> 24);
}

#endif /* TYPES_H */
