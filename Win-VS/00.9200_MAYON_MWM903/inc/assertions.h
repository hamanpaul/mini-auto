/**
 * @file assertions.h
 *      assertion macros
 * @author Jiang Yu-Kuan <york_jiang@mars-semi.com.tw>
 * @date 2013/03/15
 * @version 1.0
 */
#ifndef ASSERTIONS_H_
#define ASSERTIONS_H_

#include <stdint.h>
#include <stdio.h>


//------------------------------------------------------------------------------

/// Halt the program running.
#ifdef NOHALT_ASSERT
    #define HALT_()
#else
    #define HALT_() while (1)
#endif

/// A replacement of assert macro of C standard.
#ifndef NDEBUG
    #define assert(expr)                        \
        if (!(expr)) {                          \
            printf("[ERR] Assert failed: ");    \
            printf("%s", #expr);                \
            printf(" (file %s line %d)\n", __FILE__, (int)__LINE__ ); \
            HALT_();                            \
        }
#else
    #define	assert(expr)
#endif

//------------------------------------------------------------------------------

/// Asserts that "a op b" satisfied
#define ASSERT_OP(a, op, b)                     \
    if (!(a op b)) {                            \
        printf("[ERR] Assert failed: ");        \
        printf("%s %s %s; %s:%u, %s:%u",        \
                #a, #op, #b, #a, (uint32_t)(a), #b, (uint32_t)(b)); \
        printf(" (file %s line %d)\n",  __FILE__, (int)__LINE__ );  \
        HALT_();                                \
    }

/// Asserts that a > b
#define ASSERT_GT(a, b) ASSERT_OP(a, >, b)

/// Asserts that a >= b
#define ASSERT_GE(a, b) ASSERT_OP(a, >=, b)

/// Asserts that a < b
#define ASSERT_LT(a, b) ASSERT_OP(a, <, b)

/// Asserts that a <= b
#define ASSERT_LE(a, b) ASSERT_OP(a, <=, b)

/// Asserts that a == b
#define ASSERT_EQ(a, b) ASSERT_OP(a, ==, b)

/// Asserts that a != b
#define ASSERT_NE(a, b) ASSERT_OP(a, !=, b)

//------------------------------------------------------------------------------

/// macro for compile-time assertions
#define cassert(exp) extern char _cassert[exp]

//------------------------------------------------------------------------------


#endif  // ASSERTIONS_H_
