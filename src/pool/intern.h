/*
  +----------------------------------------------------------------------+
  | ext-tui: String interning pool                                       |
  +----------------------------------------------------------------------+
  | Reduces memory usage and speeds up comparisons by storing unique     |
  | copies of commonly used strings (keys, IDs) and returning pointers   |
  | to the interned version.                                             |
  |                                                                       |
  | Benefits:                                                             |
  |   - Memory savings: single copy of each unique string                 |
  |   - Faster comparisons: pointer equality instead of strcmp()          |
  |   - Reduced allocations: reuse existing strings                       |
  |                                                                       |
  | Thread Safety: NOT thread-safe. The intern pool is per-request and   |
  | should only be accessed from the main thread.                         |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_INTERN_H
#define TUI_INTERN_H

#include <stdint.h>
#include <stddef.h>

/* Pool configuration */
#define INTERN_POOL_BUCKETS 256      /* Hash table buckets */
#define INTERN_MAX_STRING_LEN 256    /* Maximum string length to intern */

/**
 * Interned string entry in the hash table.
 */
typedef struct tui_intern_entry {
    char *str;                         /* Interned string (heap allocated) */
    size_t len;                        /* String length (not including NUL) */
    uint32_t hash;                     /* Cached hash value */
    int refcount;                      /* Reference count */
    struct tui_intern_entry *next;     /* Next entry in bucket (collision chain) */
} tui_intern_entry;

/**
 * String interning pool.
 */
typedef struct tui_intern_pool {
    tui_intern_entry *buckets[INTERN_POOL_BUCKETS];  /* Hash table buckets */
    int total_strings;                 /* Total unique strings interned */
    int64_t total_bytes;               /* Total bytes used by strings */
    int64_t intern_hits;               /* Times an existing string was reused */
    int64_t intern_misses;             /* Times a new string was created */
} tui_intern_pool;

/**
 * Initialize the intern pool.
 * @param pool Pool to initialize
 * @return 0 on success, -1 on failure
 */
int tui_intern_pool_init(tui_intern_pool *pool);

/**
 * Shutdown and free all interned strings.
 * @param pool Pool to shutdown
 */
void tui_intern_pool_shutdown(tui_intern_pool *pool);

/**
 * Reset pool between requests (releases all strings).
 * @param pool Pool to reset
 */
void tui_intern_pool_reset(tui_intern_pool *pool);

/**
 * Intern a string - returns pointer to interned copy.
 * If the string is already interned, returns the existing pointer
 * and increments the reference count.
 *
 * @param pool Pool to use
 * @param str  String to intern (does not need to be NUL-terminated)
 * @param len  Length of string
 * @return Pointer to interned string, or NULL on failure
 *         (NULL input, len > INTERN_MAX_STRING_LEN, or allocation failure)
 */
const char* tui_intern(tui_intern_pool *pool, const char *str, size_t len);

/**
 * Intern a NUL-terminated string (convenience wrapper).
 * @param pool Pool to use
 * @param str  NUL-terminated string to intern
 * @return Pointer to interned string, or NULL on failure
 */
const char* tui_intern_str(tui_intern_pool *pool, const char *str);

/**
 * Release a reference to an interned string.
 * Decrements the reference count. When refcount reaches 0,
 * the entry is freed.
 *
 * @param pool Pool containing the string
 * @param str  Interned string to release (must be pointer returned by tui_intern)
 */
void tui_intern_release(tui_intern_pool *pool, const char *str);

/**
 * Check if a string is interned (for debugging/testing).
 * @param pool Pool to check
 * @param str  String pointer to check
 * @return 1 if the pointer points to an interned string, 0 otherwise
 */
int tui_intern_is_interned(tui_intern_pool *pool, const char *str);

#endif /* TUI_INTERN_H */
