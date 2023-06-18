#ifndef __DEFS_H__
#define __DEFS_H__

#include <stddef.h>
#include <stdint.h>

#define TRUE    1
#define FALSE   0

#define STR2(v) #v
#define STR(v) STR2(v)

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef size_t usize;

#endif
