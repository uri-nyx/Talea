#ifndef TYPES_H
#define TYPES_H

#define HAS_LIBSIRIUS_TYPES

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  usize;

typedef signed char  i8;
typedef signed short i16;
typedef signed int   i32;
typedef signed long  ssize;

typedef float f32;

typedef unsigned long  ptr;
typedef unsigned short dptr;

typedef i8 bool;
#define true  1
#define false 0

#define NULL ((void *)0)

struct akai_ringbuffer {
    u8   *data;
    usize size;
    usize head;
    usize tail;
};

struct akai_natural_ringbufferu8 {
    u8 *data;
    u8  head;
    u8  tail;
};

struct akai_natural_ringbufferu16 {
    u16 *data;
    u8   head;
    u8   tail;
};

struct akai_natural_ringbufferu32 {
    u32 *data;
    u8   head;
    u8   tail;
};

#endif /* TYPES_H */
