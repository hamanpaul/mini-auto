/**
 * @file numeric.h
 * @author Jiang Yu-Kuan <york_jiang@mars-semi.com.tw>
 * @date 2013/03/15 (initial)
 * @date 2018/03/08 (last revision)
 * @version 1.0
 */
#ifndef NUMERIC_H_
#define NUMERIC_H_

#include <stdint.h>
#include <stddef.h>


/// Type of an integer index beginning from 0
typedef size_t Index;

//------------------------------------------------------------------------------

/// Returns minimal one of two values
#define min(a, b)   ((a) < (b) ? (a) : (b))

/// Returns maximal one of two values
#define max(a, b)   ((a) > (b) ? (a) : (b))

//------------------------------------------------------------------------------

/// Returns the number of elements of an array
#define ElemsOfArray(x) (sizeof(x) / sizeof(x[0]))

/// Returns the value that is an integer multiple of \a m and closest to \a x.
#define BeIntMultipleOf(x, m) ((x) / (m) * (m))

/// Returns if \a x is an integer multiple of \a m
#define IsIntMultipleOf(x, m) (((x) % (m)) == 0)

/// Returns the even integral value closest to \a x.
#define BeEven(x) BeIntMultipleOf(x, 2)

/// Returns if \a x is an even number
#define IsEven(x) IsIntMultipleOf(x, 2)

//------------------------------------------------------------------------------

/// Returns the integral value that is closest to a/b (round to nearest).
#define DIV_ROUND(a, b) (((a) + (b)/2) / (b))

/// Returns the smallest integral value not less than a/b (round up).
#define DIV_CEIL(a, b)  (((a) + ((b) - 1)) / (b))

/// Returns the largest integral value not greater than a/b (round down).
#define DIV_FLOOR(a, b) ((a) / (b))

//------------------------------------------------------------------------------

/// Returns if \a x is a power of 2.
#define IS_POWER_OF_2(x)    ((x) && !((x) & ((x) - 1)))

/// Returns the mask of a bit.
#define BIT_MASK(b) (1U << (b))

/// Returns the mask of bits between a and b (a >= b).
#define MASK(a, b)  (((1U << ((a)-(b)+1)) - 1) << (b))

//------------------------------------------------------------------------------

#endif  // NUMERIC_H_
