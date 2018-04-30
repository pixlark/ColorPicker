// -*- mode: c++ -*-
/** COPYRIGHT (C) 2017
 ** https://pixlark.github.io/
 *
 ** pixint.h
 * 
 * This file holds redefinitions of basic types to increase code
 * readability and typability.
 *
 */

#ifndef PIXLIB_PIXINT_H
#define PIXLIB_PIXINT_H

#include <stdint.h>

#define s8 int8_t
#define u8 uint8_t

#define s16 int16_t
#define u16 uint16_t

#define s32 int32_t
#define u32 uint32_t

#define s64 int64_t
#define u64 uint64_t

#define f32 float
#define f64 double

/* 
   Pros:
     - Compiler spits out the correct name
   Cons:
     - Loses advantages of whatever is in stdint.h
 *//*
typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed long s32;
typedef unsigned long u32;

typedef signed long long s64;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;
*/

#endif
