/*
  +----------------------------------------------------------------------+
  | ext-tui: Debug assertion macros                                      |
  +----------------------------------------------------------------------+
  | Provides debug-only assertions for catching invariant violations.    |
  | In release builds (TUI_DEBUG not defined), these expand to nothing.  |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_DEBUG_H
#define TUI_DEBUG_H

#ifdef TUI_DEBUG

#include <stdio.h>
#include <stdlib.h>

/**
 * Assert that a condition is true.
 * In debug builds, terminates with an error message if condition is false.
 */
#define TUI_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "TUI_ASSERT failed: %s\n  at %s:%d in %s()\n", \
                    #cond, __FILE__, __LINE__, __func__); \
            abort(); \
        } \
    } while (0)

/**
 * Assert that a condition is true, with a custom message.
 * In debug builds, terminates with the message if condition is false.
 */
#define TUI_ASSERT_MSG(cond, msg) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "TUI_ASSERT failed: %s\n  Message: %s\n  at %s:%d in %s()\n", \
                    #cond, (msg), __FILE__, __LINE__, __func__); \
            abort(); \
        } \
    } while (0)

/**
 * Assert that a pointer is not NULL.
 */
#define TUI_ASSERT_NOT_NULL(ptr) \
    TUI_ASSERT_MSG((ptr) != NULL, "pointer is NULL")

/**
 * Assert that a value is within a range (inclusive).
 */
#define TUI_ASSERT_RANGE(val, min, max) \
    TUI_ASSERT_MSG((val) >= (min) && (val) <= (max), "value out of range")

/**
 * Mark a code path as unreachable.
 * In debug builds, terminates if reached.
 */
#define TUI_UNREACHABLE() \
    do { \
        fprintf(stderr, "TUI_UNREACHABLE reached at %s:%d in %s()\n", \
                __FILE__, __LINE__, __func__); \
        abort(); \
    } while (0)

/**
 * Debug-only printf.
 */
#define TUI_DEBUG_PRINT(...) \
    fprintf(stderr, "[TUI DEBUG] " __VA_ARGS__)

#else /* !TUI_DEBUG */

/* In release builds, all debug macros expand to nothing */
#define TUI_ASSERT(cond)              ((void)0)
#define TUI_ASSERT_MSG(cond, msg)     ((void)0)
#define TUI_ASSERT_NOT_NULL(ptr)      ((void)0)
#define TUI_ASSERT_RANGE(val, min, max) ((void)0)
#define TUI_UNREACHABLE()             ((void)0)
#define TUI_DEBUG_PRINT(...)          ((void)0)

#endif /* TUI_DEBUG */

#endif /* TUI_DEBUG_H */
