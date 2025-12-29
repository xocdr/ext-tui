/*
  +----------------------------------------------------------------------+
  | ext-tui: Object pools implementation                                |
  +----------------------------------------------------------------------+
*/

#include "../node/reconciler.h"  /* Must come first for tui_diff_op definition */
#include "../node/node.h"
#include "pool.h"
#include <stdlib.h>
#include <string.h>

/*
 * Pool initialization
 */
int tui_pools_init(tui_pools *pools)
{
    if (!pools) return -1;

    memset(pools, 0, sizeof(tui_pools));

    /* Allocate diff operation slab */
    pools->diff_ops.ops = calloc(DIFF_OP_POOL_SIZE, sizeof(tui_diff_op));
    if (!pools->diff_ops.ops) return -1;

    pools->diff_ops.in_use = calloc(DIFF_OP_POOL_SIZE, sizeof(uint8_t));
    if (!pools->diff_ops.in_use) {
        free(pools->diff_ops.ops);
        pools->diff_ops.ops = NULL;
        return -1;
    }

    pools->diff_ops.capacity = DIFF_OP_POOL_SIZE;
    pools->diff_ops.free_count = DIFF_OP_POOL_SIZE;
    pools->diff_ops.next_free_hint = 0;

    /* Children arrays are allocated on demand, nothing to init */

    /* Key map allocated on demand */
    pools->key_map.entries = NULL;
    pools->key_map.capacity = 0;
    pools->key_map.in_use = 0;

    return 0;
}

void tui_pools_shutdown(tui_pools *pools)
{
    if (!pools) return;

    /* Free diff operation slab */
    free(pools->diff_ops.ops);
    free(pools->diff_ops.in_use);
    pools->diff_ops.ops = NULL;
    pools->diff_ops.in_use = NULL;

    /* Free any pooled children arrays */
    for (int i = 0; i < pools->children.count_4; i++) {
        free(pools->children.arrays_4[i]);
    }
    for (int i = 0; i < pools->children.count_8; i++) {
        free(pools->children.arrays_8[i]);
    }
    for (int i = 0; i < pools->children.count_16; i++) {
        free(pools->children.arrays_16[i]);
    }
    for (int i = 0; i < pools->children.count_32; i++) {
        free(pools->children.arrays_32[i]);
    }

    /* Free key map */
    free(pools->key_map.entries);
    pools->key_map.entries = NULL;
}

void tui_pools_reset(tui_pools *pools)
{
    if (!pools) return;

    /* Reset diff operation pool - mark all as free */
    if (pools->diff_ops.in_use) {
        memset(pools->diff_ops.in_use, 0, DIFF_OP_POOL_SIZE * sizeof(uint8_t));
    }
    pools->diff_ops.free_count = DIFF_OP_POOL_SIZE;
    pools->diff_ops.next_free_hint = 0;

    /* Don't free children arrays - they can be reused across requests */

    /* Key map can be reused */
    pools->key_map.in_use = 0;
}

/*
 * Diff operation pool implementation
 */
tui_diff_op* tui_diff_op_pool_alloc(tui_pools *pools)
{
    if (!pools || !pools->diff_ops.ops) {
        /* Fallback to malloc */
        return calloc(1, sizeof(tui_diff_op));
    }

    /* Try to find a free slot starting from hint */
    if (pools->diff_ops.free_count > 0) {
        int start = pools->diff_ops.next_free_hint;
        for (int i = 0; i < DIFF_OP_POOL_SIZE; i++) {
            int idx = (start + i) % DIFF_OP_POOL_SIZE;
            if (!pools->diff_ops.in_use[idx]) {
                pools->diff_ops.in_use[idx] = 1;
                pools->diff_ops.free_count--;
                pools->diff_ops.next_free_hint = (idx + 1) % DIFF_OP_POOL_SIZE;
                pools->diff_ops_allocated++;

                /* Clear the operation */
                memset(&pools->diff_ops.ops[idx], 0, sizeof(tui_diff_op));
                return &pools->diff_ops.ops[idx];
            }
        }
    }

    /* Pool exhausted, fallback to malloc */
    pools->diff_ops_fallback++;
    return calloc(1, sizeof(tui_diff_op));
}

void tui_diff_op_pool_free(tui_pools *pools, tui_diff_op *op)
{
    if (!op) return;

    if (!pools || !pools->diff_ops.ops) {
        /* Was allocated via malloc */
        free(op);
        return;
    }

    /* Check if this op is in our pool */
    ptrdiff_t offset = op - pools->diff_ops.ops;
    if (offset >= 0 && offset < DIFF_OP_POOL_SIZE) {
        /* It's in our pool */
        int idx = (int)offset;
        if (pools->diff_ops.in_use[idx]) {
            pools->diff_ops.in_use[idx] = 0;
            pools->diff_ops.free_count++;
            pools->diff_ops_reused++;

            /* Update hint for faster next allocation */
            if (idx < pools->diff_ops.next_free_hint) {
                pools->diff_ops.next_free_hint = idx;
            }
        }
    } else {
        /* Was allocated via malloc (fallback) */
        free(op);
    }
}

void tui_diff_op_pool_free_all(tui_pools *pools, tui_diff_op *ops, int count)
{
    if (!ops || count <= 0) return;

    if (!pools || !pools->diff_ops.ops) {
        /* Ops array was malloc'd as a block, not from pool */
        free(ops);
        return;
    }

    /* Check if the ops array is our pool's slab */
    ptrdiff_t offset = ops - pools->diff_ops.ops;
    if (offset >= 0 && offset < DIFF_OP_POOL_SIZE) {
        /* These are from our pool - mark all as free */
        for (int i = 0; i < count && (offset + i) < DIFF_OP_POOL_SIZE; i++) {
            int idx = (int)(offset + i);
            if (pools->diff_ops.in_use[idx]) {
                pools->diff_ops.in_use[idx] = 0;
                pools->diff_ops.free_count++;
                pools->diff_ops_reused++;
            }
        }
        pools->diff_ops.next_free_hint = (int)offset;
    } else {
        /* Was allocated via malloc as a separate array */
        free(ops);
    }
}

/*
 * Children array pool implementation
 */
struct tui_node** tui_children_pool_alloc(tui_pools *pools, int capacity, int *actual_capacity)
{
    struct tui_node **array = NULL;

    if (!pools) {
        /* Fallback to malloc */
        *actual_capacity = capacity;
        return calloc(capacity, sizeof(struct tui_node*));
    }

    /* Find the smallest size class that fits */
    if (capacity <= 4) {
        if (pools->children.count_4 > 0) {
            array = pools->children.arrays_4[--pools->children.count_4];
            *actual_capacity = 4;
            pools->children_allocated++;
            memset(array, 0, 4 * sizeof(struct tui_node*));
            return array;
        }
        /* Allocate new array of size 4 */
        *actual_capacity = 4;
    } else if (capacity <= 8) {
        if (pools->children.count_8 > 0) {
            array = pools->children.arrays_8[--pools->children.count_8];
            *actual_capacity = 8;
            pools->children_allocated++;
            memset(array, 0, 8 * sizeof(struct tui_node*));
            return array;
        }
        *actual_capacity = 8;
    } else if (capacity <= 16) {
        if (pools->children.count_16 > 0) {
            array = pools->children.arrays_16[--pools->children.count_16];
            *actual_capacity = 16;
            pools->children_allocated++;
            memset(array, 0, 16 * sizeof(struct tui_node*));
            return array;
        }
        *actual_capacity = 16;
    } else if (capacity <= 32) {
        if (pools->children.count_32 > 0) {
            array = pools->children.arrays_32[--pools->children.count_32];
            *actual_capacity = 32;
            pools->children_allocated++;
            memset(array, 0, 32 * sizeof(struct tui_node*));
            return array;
        }
        *actual_capacity = 32;
    } else {
        /* Too large for pool */
        *actual_capacity = capacity;
        pools->children_fallback++;
        return calloc(capacity, sizeof(struct tui_node*));
    }

    /* Need to allocate new array */
    pools->children_fallback++;
    return calloc(*actual_capacity, sizeof(struct tui_node*));
}

void tui_children_pool_free(tui_pools *pools, struct tui_node **array, int capacity)
{
    if (!array) return;

    if (!pools) {
        free(array);
        return;
    }

    /* Try to return to appropriate pool */
    if (capacity == 4 && pools->children.count_4 < CHILDREN_POOL_SIZE_4) {
        pools->children.arrays_4[pools->children.count_4++] = array;
        pools->children_reused++;
        return;
    } else if (capacity == 8 && pools->children.count_8 < CHILDREN_POOL_SIZE_8) {
        pools->children.arrays_8[pools->children.count_8++] = array;
        pools->children_reused++;
        return;
    } else if (capacity == 16 && pools->children.count_16 < CHILDREN_POOL_SIZE_16) {
        pools->children.arrays_16[pools->children.count_16++] = array;
        pools->children_reused++;
        return;
    } else if (capacity == 32 && pools->children.count_32 < CHILDREN_POOL_SIZE_32) {
        pools->children.arrays_32[pools->children.count_32++] = array;
        pools->children_reused++;
        return;
    }

    /* Pool full or non-standard size, free it */
    free(array);
}

/*
 * Key map pool implementation
 *
 * We store a raw byte array that can be reused for key map entries.
 * The entry_size is passed in by the caller to handle proper alignment.
 */

/* Track the entry size for proper clearing */
static size_t g_key_map_entry_size = 0;

void* tui_key_map_pool_acquire(tui_pools *pools, int initial_capacity, size_t entry_size)
{
    if (!pools) {
        return calloc(initial_capacity, entry_size);
    }

    /* Store entry size for clearing on reuse */
    g_key_map_entry_size = entry_size;

    if (pools->key_map.in_use) {
        /* Already in use, allocate new one */
        return calloc(initial_capacity, entry_size);
    }

    if (pools->key_map.entries && pools->key_map.capacity >= initial_capacity) {
        /* Reuse existing */
        pools->key_map.in_use = 1;
        pools->key_map_reused++;
        memset(pools->key_map.entries, 0, pools->key_map.capacity * entry_size);
        return pools->key_map.entries;
    }

    /* Need larger or first allocation */
    free(pools->key_map.entries);
    pools->key_map.entries = calloc(initial_capacity, entry_size);
    pools->key_map.capacity = initial_capacity;
    pools->key_map.in_use = 1;
    return pools->key_map.entries;
}

void tui_key_map_pool_release(tui_pools *pools)
{
    if (pools) {
        pools->key_map.in_use = 0;
    }
}
