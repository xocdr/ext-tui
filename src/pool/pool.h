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
struct tui_diff_op;
typedef struct tui_diff_op tui_diff_op;

/*
 * Diff Operation Pool
 * Pre-allocates a slab of diff operations to avoid malloc per operation.
 */
#define DIFF_OP_POOL_SIZE 256

typedef struct {
    tui_diff_op *ops;           /* Slab of operations */
    uint8_t *in_use;            /* Bitmap of which slots are in use */
    int capacity;               /* Total slots (DIFF_OP_POOL_SIZE) */
    int free_count;             /* Number of free slots */
    int next_free_hint;         /* Hint for O(1) allocation */
} tui_diff_op_pool;

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
typedef struct {
    tui_diff_op_pool diff_ops;
    tui_children_pool children;
    tui_key_map_pool key_map;

    /* Pool metrics (only collected when metrics enabled) */
    int64_t diff_ops_allocated;     /* Total allocations from pool */
    int64_t diff_ops_fallback;      /* Allocations that fell back to malloc */
    int64_t diff_ops_reused;        /* Times an op was returned to pool */
    int64_t children_allocated;     /* Total children array allocations */
    int64_t children_fallback;      /* Children arrays from malloc */
    int64_t children_reused;        /* Children arrays returned to pool */
    int64_t key_map_reused;         /* Key map reuses */
} tui_pools;

/* Pool lifecycle (called from MINIT/MSHUTDOWN) */
int tui_pools_init(tui_pools *pools);
void tui_pools_shutdown(tui_pools *pools);

/* Reset pools between requests (called from RINIT) */
void tui_pools_reset(tui_pools *pools);

/*
 * Diff operation pool API
 */

/* Allocate a diff operation (from pool or malloc if exhausted) */
tui_diff_op* tui_diff_op_pool_alloc(tui_pools *pools);

/* Return a diff operation to the pool */
void tui_diff_op_pool_free(tui_pools *pools, tui_diff_op *op);

/* Bulk free all operations in a diff result */
void tui_diff_op_pool_free_all(tui_pools *pools, tui_diff_op *ops, int count);

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
