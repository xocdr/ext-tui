/*
  +----------------------------------------------------------------------+
  | ext-tui: Object pools for memory optimization                       |
  +----------------------------------------------------------------------+
  | Reduces malloc/free churn by reusing frequently allocated objects.  |
  | Pools are thread-local in ZTS builds via module globals.            |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_POOL_H
#define TUI_POOL_H

#include <stdint.h>

/* Forward declarations */
struct tui_node;

/*
 * Children Array Pool
 * Pools arrays of common sizes (4, 8, 16, 32) for node children.
 */
#define CHILDREN_POOL_SIZE_4   16
#define CHILDREN_POOL_SIZE_8   16
#define CHILDREN_POOL_SIZE_16  8
#define CHILDREN_POOL_SIZE_32  4

typedef struct {
    struct tui_node **arrays_4[CHILDREN_POOL_SIZE_4];
    struct tui_node **arrays_8[CHILDREN_POOL_SIZE_8];
    struct tui_node **arrays_16[CHILDREN_POOL_SIZE_16];
    struct tui_node **arrays_32[CHILDREN_POOL_SIZE_32];
    int count_4;
    int count_8;
    int count_16;
    int count_32;
} tui_children_pool;

/*
 * Key Map Pool
 * Single reusable key map cleared between reconciliation runs.
 */
typedef struct {
    void *entries;              /* key_map_entry array */
    int capacity;
    int in_use;
} tui_key_map_pool;

/*
 * Combined pool structure for module globals
 */
typedef struct tui_pools {
    tui_children_pool children;
    tui_key_map_pool key_map;

    /* Pool metrics */
    int64_t children_hits;          /* Successful pool allocations */
    int64_t children_misses;        /* Allocations that missed pool (new alloc or too large) */
    int64_t children_returns;       /* Arrays returned to pool */
    int64_t key_map_reuses;         /* Key map reuses */
} tui_pools;

/* Pool lifecycle (called from MINIT/MSHUTDOWN) */
int tui_pools_init(tui_pools *pools);
void tui_pools_shutdown(tui_pools *pools);

/* Reset pools between requests (called from RINIT) */
void tui_pools_reset(tui_pools *pools);

/*
 * Children array pool API
 */

/* Allocate array of at least `capacity` elements, returns actual capacity */
struct tui_node** tui_children_pool_alloc(tui_pools *pools, int capacity, int *actual_capacity);

/* Return array to pool (capacity must match what was allocated) */
void tui_children_pool_free(tui_pools *pools, struct tui_node **array, int capacity);

/*
 * Key map pool API
 */

/* Acquire the shared key map (cleared, ready for use) */
void* tui_key_map_pool_acquire(tui_pools *pools, int initial_capacity, size_t entry_size);

/* Release the key map back to pool */
void tui_key_map_pool_release(tui_pools *pools);

#endif /* TUI_POOL_H */
