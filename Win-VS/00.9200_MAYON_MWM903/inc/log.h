/**
 * @file log.h
 *      a Log System implemented with only C macros
 * @author Jiang Yu-Kuan <york_jiang@mars-semi.com.tw>
 * @date 2013/03/15
 * @version 1.0
 */
#ifndef LOG_H_
#define LOG_H_


#include <stdio.h>


//-----------------------------------------------------------------------------

#define LL_TRACE 4      ///< log level of code tracing
#define LL_DEBUG 3      ///< log level of code debug
#define LL_INFO  2      ///< log level of printout normal infomation
#define LL_ERROR 1      ///< log level of error messages
#define LL_OFF   0      ///< turn off log messages

/// Sets the default log level.
#if !defined(LOG_LEVEL)
    #define LOG_LEVEL   LL_INFO
#endif

/// The common log function.
#define LOG(tag, args)  \
    do {                \
        printf(tag);    \
        printf args;    \
        printf("\n");   \
    } while (0)

//-----------------------------------------------------------------------------

/// log for tracing code
#if LOG_LEVEL >= LL_TRACE
    #define TRC(args)  LOG("[TRC] ", args)
#else
    #define TRC(args)
#endif

/// log for debug
#if LOG_LEVEL >= LL_DEBUG
    #define DBG(args)  LOG("[DBG] ", args)
#else
    #define DBG(args)
#endif

/// log for normal infomation
#if LOG_LEVEL >= LL_INFO
    #define INF(args)  LOG("[INF] ", args)
#else
    #define INF(args)
#endif

/// log for error messages
#if LOG_LEVEL >= LL_ERROR
    #define ERR(args)  LOG("[ERR] ", args)
#else
    #define ERR(args)
#endif

//-----------------------------------------------------------------------------

/// Code block for tracing code
#if LOG_LEVEL >= LL_TRACE
    #define TraceCode(statements)   {statements}
#else
    #define TraceCode(statements)
#endif

/// Code block for debug
#if LOG_LEVEL >= LL_DEBUG
    #define DebugCode(statements)   {statements}
#else
    #define DebugCode(statements)
#endif

//-----------------------------------------------------------------------------

// Just printf
#if LOG_LEVEL >= LL_ERROR
#define ERRD(expr) do{	\
        ERR(("%s (file %s line %d)\n", #expr, __FILE__, (int)__LINE__));	\
        HALT_();	\
    }while(0)
#else
#define	ERRD(expr)
#endif

//------------------------------------------------------------------------------



#endif  // LOG_H_
